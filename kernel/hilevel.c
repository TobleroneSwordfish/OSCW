/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */


#include "hilevel.h"

extern void     main_P3();
extern void     main_P4();
extern void     main_P5();
extern void     main_P6();
extern void main_console_custom();
extern int write( int fd, const void* x, size_t n );

#define proc_no 100
#define STACK_SIZE 0x10000

#define MAX_FILES 20
#define FILESIZE 1000

//files are stored directly at index fd where fd is their file descriptor
//to allow for O(1) access time
file_t files[MAX_FILES];

uint32_t last_file_address = 1000;
//initialised to 3 for the 3 system fds defined in libc
int last_fd = 3;

directory_t root_dir;

int block_count;
int block_length;

pcb_t pcb[ proc_no ]; pcb_t* current = NULL;

//PIDs are 1 indexed, I know, forgive me father for I have sinned
pid_t last_pid = 0;

pid_t semaphore[100];

pcb_t* make_proc(int priority, void* main)
{
    int index = last_pid++;
    void* bos = malloc(STACK_SIZE);
	if (bos == NULL)
	{
		PL011_putc(UART0, 'X', true);
	}
    memset( &pcb[ index ], 0, sizeof( pcb_t ) );     // initialise PCB
    pcb[ index ].pid      = index + 1;
    pcb[ index ].status   = STATUS_CREATED;
    pcb[ index ].age = 0;
    pcb[ index ].priority = priority;
    pcb[ index ].bos = (uint32_t)bos;
    pcb[ index ].main = ( uint32_t )main;
    pcb[ index ].ctx.cpsr = 0x50;
    pcb[ index ].ctx.pc   = ( uint32_t )( main );
    pcb[ index ].ctx.sp   = ( uint32_t )( bos ) + STACK_SIZE;
    pcb[ index ].return_value = -1;
    pcb[ index ].waitingon = -1;
    return &pcb[ index ];
}

directory_t* make_dir(directory_t* parent, char* name)
{
    directory_t* newdir = malloc(sizeof(directory_t));
    parent->dirs[parent->dir_count++] = newdir;
    newdir->name = strdup(name);
    newdir->file_count = 0;
    newdir->dir_count = 0;
    return newdir;
}

file_t* make_file(directory_t* parent, char* name)
{
    int fd = ++last_fd;
    file_t* newfile = &files[fd];
    newfile->name = strdup(name);
    last_file_address += FILESIZE;
    newfile->address = last_file_address;
    newfile->length = 0;
    newfile->fd = fd;
    parent->files[parent->file_count] = newfile;
    parent->file_count++;
    return newfile;
}

void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {
  char prev_pid = '?', next_pid = '?';

  if( NULL != prev ) {
    memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    PL011_putc( UART0, '[',      true );
    PL011_putc( UART0, prev_pid, true );
    PL011_putc( UART0, '-',      true );
    PL011_putc( UART0, '>',      true );
    PL011_putc( UART0, next_pid, true );
    PL011_putc( UART0, ']',      true );

    if (ctx->sp > next->bos + STACK_SIZE || ctx->sp < next->bos)
    {
        char* error_string = "Catastrophic stack error\n";
        for (int i = 0; i < strlen(error_string); i++)
        {
            PL011_putc(UART0, error_string[i], true);
        }
    }
    
    current = next;                             // update   executing index   to P_{next}
  return;
}

void schedule( ctx_t* ctx ) {
  if (!(current->pid > 0 && current->pid <= last_pid))
  {
      PL011_putc(UART0, 'e', true);
      PL011_putc(UART0, '0' + current->pid, true);
  }
  volatile int next_pid = -1;
  int best_prio = 0;
  for (int i = 0; i < last_pid; i++)
  {
      //process not ded or sleeping
      if (pcb[i].status != STATUS_TERMINATED && pcb[i].status != STATUS_WAITING)
      {
          //composite priority of base priority and age
          int p = pcb[i].priority + pcb[i].age;
          if (p > best_prio)
          {
              best_prio = p;
              next_pid = i + 1;
          }
      }
      pcb[i].age++;
  }
  //don't reanimate dead processes, necromancy is banned
  if (current->status == STATUS_EXECUTING)
  {
      pcb[current->pid - 1].status = STATUS_READY;
  }
  pcb[next_pid - 1].status = STATUS_EXECUTING;
  pcb[next_pid - 1].age = 0;
  if (next_pid != current->pid)
  {
      dispatch(ctx, &pcb[current->pid - 1], &pcb[next_pid - 1]);
  }

  return;
}

//sets a process to terminated, frees the stack, and returns control to the waiting parent process is applicable
void kill_proc(pcb_t* p, ctx_t* c)
{
    p->status = STATUS_TERMINATED;
    if (p->pid == current->pid)
    {
        current->status = STATUS_TERMINATED;
    }
    free((void*)p->bos);//no memory leaks here boss
	free(p->args);
    pcb_t* parent = &pcb[p->parent - 1];
    if (parent->status == STATUS_WAITING && parent->waitingon == p->pid)
    {
        parent->status = STATUS_EXECUTING;
        parent->waitingon = -1;
        dispatch(c, current, parent); //return control to waiting parent process
    }
    else if (p->pid == current->pid)
    {
        schedule(c);
    }
}

void hilevel_handler_rst(ctx_t* ctx) {

  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each 
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */
	

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  for (int i = 0; i < 100; i++)
  {
      semaphore[i] = 0;
  }
  
  block_length = disk_get_block_len();
  block_count = disk_get_block_num();
  
  //we have to initialise the root directory manually
  root_dir.name = "root";
  root_dir.file_count = 0;
  root_dir.dir_count = 0;  
  
  //some test files
  make_file(&root_dir, "test"); 
  make_file(&root_dir, "test2");
  
  directory_t* other_dir = make_dir(&root_dir, "user");

  make_file(other_dir, "test3");

  //start the console process  
  make_proc(1, &main_console_custom);
  
  dispatch( ctx, NULL, &pcb[ 0 ] );
    
  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  //read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  //handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    //PL011_putc( UART0, 'T', true );
    schedule(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  //write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;
  return;
}

//warning: bolognese below
void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  switch (id)
  {
    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      uint8_t*  x = ( uint8_t* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 
      if (fd <= 3) //write to console
      {
        PL011_t* channel;
        if (fd == 1) //emulation console
        {
            channel = UART0;
        }
        if (fd == 3) //user console
        {
            channel = UART1;
        }
        for( int i = 0; i < n; i++ ) {
          PL011_putc( channel, *x++, true );
        }
      }
      else if (fd > 3)
      {
          uint32_t address = files[fd].address;
          //set the length as we're just block overwriting rn
          files[fd].length = n;
          int offset = 0;
          //write all whole blocks
          while(offset <= n - block_length)
          {
              disk_wr(address + offset, x + offset, block_length);
              offset += block_length;
          }
          //write the remaining bytes
          if (offset != n)
          {
              uint8_t* data = malloc(sizeof(uint8_t) * block_length);
              for (int i = 0; i < block_length; i++)
              {
                  //copy remaining data
                  if (i < n - offset)
                  {
                      data[i] = *(x + offset + i);
                  }
                  else
                  {
                      //pad with zeroes
                      data[i] = 0;
                  }
              }
              disk_wr(address + offset, data, block_length);
              free(data);
          }
      }
      ctx->gpr[ 0 ] = n;

      break;
    }
    case 0x02: //book
    {
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      uint8_t*  x = ( uint8_t* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 
      
      if (n < 0)
      {
          n = files[fd].length;
      }
        
      uint32_t address = files[fd].address;
      
      int offset = 0;
      
      while(offset <= n)
      {
          //room to copy a full block
          if (offset + block_length < n)
          {
              disk_rd(address + offset, x + offset, block_length);
          }
          else //have to read to an intermediate array
          {
              uint8_t* data = malloc(sizeof(uint8_t) * block_length);
              disk_rd(address + offset, data, block_length);
              memcpy(x + offset, data, n - offset);
              free(data);
          }
          offset += block_length;
      }
      ctx->gpr[0] = n;
      break;
    }
    case 0x03: //spoon
    {
      pcb_t* new = make_proc(current->priority, (void*)current->main);
      memcpy((void*)new->bos, (void*)current->bos, STACK_SIZE); //clone stack
      new->ctx.pc = ctx->pc;
      new->ctx.cpsr = ctx->cpsr;
      new->ctx.lr = ctx->lr;
      memcpy(new->ctx.gpr, ctx->gpr, sizeof(uint32_t) * 13);
      volatile uint32_t offset = ctx->sp - current->bos;
      new->ctx.sp = new->bos + offset; //correct stack pointer for new stack location
      new->parent = current->pid; //set parent of child
      ctx->gpr[0] = new->pid; //return PID of child
      new->ctx.gpr[0] = 0; //return 0 to child
      break;
    }
    case 0x04: //go commit lens self damage
    {
      PL011_putc(UART0, 'E', true);
      volatile int value = ctx->gpr[0];
      current->return_value = value;
      kill_proc(current, ctx);
      break;
    }
    case 0x05: //66
    {
      void* main = (void*)(ctx->gpr[0]);
      char** args = (char**)(ctx->gpr[1]);
	  int n = (int)(ctx->gpr[2]);
      if (main == NULL)
      {
        exit(0);
        break;
      }
      if (args != NULL)
      {
		  current->args = malloc(sizeof(char*) * n);
		  current->arg_no = n;
          for (int i = 0; i < n; i++)
		  {
			  current->args[i] = strdup(args[i]);
		  }
      }
      current->main = (uint32_t)main;
      ctx->pc = (uint32_t)main;
      break;
    }
    case 0x06: //murrdurr
    {
      pid_t pid = ctx->gpr[0];
      kill_proc(&pcb[pid - 1], ctx);
      break;
    }
    case 0x07: //noice
    {
        pid_t pid = ctx->gpr[0];
        int increment = ctx->gpr[1];
        pcb[pid - 1].priority -= increment;
        break;
    }
    case 0x08: //manifest destiny
    {
      uint32_t offset = ctx->gpr[0];
      uint32_t claims = ctx->gpr[1];
      char success = 1;
      //check that the indexes are clear
      for (int i = 0; i < 32 && success; i++)
      {
        char index = offset + i;
        char value = (claims >> i) & 1;
        if (value && semaphore[index] != 0)
        {
          ctx->gpr[0] = -1;
          success = 0;
        }
      }
      if (success)
      {
        //actually set the values
        for (int i = 0; i < 32; i++)
        {
          char value = (claims >> i) & 1;
          if (value)
          {
            char index = offset + i;
            semaphore[index] = current->pid;
          }
        }
        ctx->gpr[0] = 0;
      }
      break;
    }
    case 0x09: //let go
    {
      uint32_t offset = ctx->gpr[0];
      uint32_t claims = ctx->gpr[1];
      for (int i = 0; i < 32; i++)
      {
        //trying to release it and have permission to
        if(((claims >> i) & 1) && semaphore[offset + i] == current->pid)
        {
          semaphore[offset + i] = 0;
        }
      }
      break;
    }
    case 0x0A: //I sleep
    {
      current->status = STATUS_WAITING;
      pid_t pid = ctx->gpr[0];
      //he's already dead
      if (pcb[pid - 1].status == STATUS_TERMINATED)
      {
        current->status = STATUS_EXECUTING;
        break;
      }
      current->waitingon = pid;
      schedule(ctx);
      break;
    }
    case 0x0B: //watchman
    {
      pid_t pid = ctx->gpr[0];
      ctx->gpr[0] = pcb[pid - 1].return_value;
      break;
    }
    case 0x0C: //sesame
    {
      //path string in the format "dir/dir2/dir3/filename" with a string terminator
      char* path = (char*)ctx->gpr[0];
      //where we've got to
      directory_t* current_dir = &root_dir;
      
      char* delimiter = "/";
      char* section = strtok(path, delimiter);
      char* next_section = strtok(NULL, delimiter);
      while(next_section != NULL)
      {
          char found = 0;
          for (int i = 0; i < current_dir->dir_count && !found; i++)
          {
              if (strcmp(section, current_dir->dirs[i]->name) == 0)
              {
                  current_dir = current_dir->dirs[i];
                  found = true;
              }
          }
          if (!found)
          {
              current_dir = make_dir(current_dir, section);
          }
          strcpy(section, next_section);
          next_section = strtok(NULL, delimiter);
      }
      char found = 0;
      for (int i = 0; i < current_dir->file_count && !found; i++)
      {
          if (strcmp(section, current_dir->files[i]->name) == 0)
          {
              ctx->gpr[0] = current_dir->files[i]->fd;
              found = true;
          }
      }
      if (!found)
      {
          file_t* file = make_file(current_dir, section);
          ctx->gpr[0] = file->fd;
      }
      break;
    }
    case 0x0D: //argh
    {
        int i = (int)ctx->gpr[0];
		if (i >= 0 && i < current->arg_no)
		{
			ctx->gpr[0] = (uint32_t)current->args[i];
		}
		else
		{
			ctx->gpr[0] = (uint32_t)NULL;
		}
        break;
    }
    case 0x0E: //bigness
    {
        int fd = (int)ctx->gpr[0];
        ctx->gpr[0] = (uint32_t)files[fd].length;
        break;
    }
    default: {
      //explode
      break;  
    }
  }
  return;
}

// void store_dir(directory_t* dir, int address)
// {
    
// }