/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include "string.h"
#include "stdio.h"
#include <sys/types.h>
#include "MMU.h"

extern uint32_t end;
int pid_n = 30;

extern void main_console();
extern uint32_t tos_console;
extern uint32_t tos_mutex;
extern void main_P3();
extern void main_P4();
extern void main_P5();
extern void main_PX();

pcb_t pcb[30];
char executing = '0';
pcb_t *current = NULL;
int n = 1; //Total number of active process
mutex (*mutexes)[30] = (void*)&tos_mutex;


caddr_t _sbrk(int incr) {
  return (caddr_t)(&end);
}


void init_pcb(){
  for(int i = 0; i<30; i++){
    pcb[i].pid = -1;
  }
}

void init_mutx(){
  for(int i = 0; i<30; i++){
    mutexes[i]->status = MUTX_UNLOCKED;
    mutexes[i]->owner  = 0;
    mutexes[i]->data   = 0;
  }
}

int get_freepcb(){
  for(int i = 0; i<sizeof(pcb); i++){
    if (pcb[i].pid == -1){
      // PL011_putc(UART0, 'Q', true);
      // PL011_putc(UART0, '0'+i, true);
      return i;
      break;
    }
  }
}

//TODO: Print out double digits
void printt(pid_t n){
  
}


void dispatch(ctx_t *ctx, pcb_t *prev, pcb_t *next) {
  char prev_pid = '?', next_pid = '?';

  if (NULL != prev){
    memcpy(&prev->ctx, ctx, sizeof(ctx_t)); // preserve execution context of P_{prev}
    prev->status = STATUS_READY;
    prev_pid = '0' + prev->pid;
  }
  if (NULL != next){
    memcpy(ctx, &next->ctx, sizeof(ctx_t)); // restore  execution context of P_{next}
    next->status = STATUS_EXECUTING;
    next_pid = '0' + next->pid;
  }


  PL011_putc(UART0, '[', true);
  PL011_putc(UART0, prev_pid, true);
  PL011_putc(UART0, '-', true);
  PL011_putc(UART0, '>', true);
  PL011_putc(UART0, next_pid, true);
  PL011_putc(UART0, ']', true);

  executing = next_pid;
  // PL011_putc(UART0, executing, true);

  prev->runtime = prev->runtime + 1; //Increase runtime
  current = next; // update   executing index   to P_{next}

  return;
}

uint32_t get_memloc(int pid){
  return (uint32_t)(&tos_console + (pid -1 )*(0x00001000));
}

void hilevel_handler_rst(ctx_t *ctx) {

  uint8_t *x = (uint8_t *)(malloc(10));
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each 
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  init_pcb();
  init_mutx();

  memset(&pcb[0], 0, sizeof(pcb_t)); // initialise 0-th PCB = P_1
  pcb[0].pid = 1;
  pcb[0].priority = 1;
  pcb[0].runtime = 0;
  pcb[0].status = STATUS_READY;
  pcb[0].ctx.cpsr = 0x50;
  pcb[0].ctx.pc = (uint32_t)(&main_console);
  pcb[0].ctx.sp = (uint32_t)(&tos_console);

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();

  dispatch(ctx, NULL, &pcb[0]);
  return;
}

void schedule(ctx_t* ctx){

  for(int i=0; i<n; i++){

    if(current->pid == pcb[i].pid && current->status == STATUS_EXECUTING){
      int r = i + 1;
      if((r % n) != i){
        while (pcb[r % n].status != STATUS_READY &&(r % n != i)){
          r++;
        }
      }

      dispatch(ctx, &pcb[i], &pcb[r % n]);
      break;
    }

    if (current->pid == pcb[i].pid && current->status == STATUS_TERMINATED){
      int j = i + 1;

      while (pcb[j % n].status != STATUS_READY){
          j++;
      }

      dispatch(ctx, NULL, &pcb[j % n]);
      break;
    }
  }
  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the interrupt source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    schedule(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;
  return;
}

void hilevel_handler_svc(ctx_t *ctx, uint32_t id){
  /* Based on the identifier (i.e., the immediate operand) extracted from the
   * svc instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call, then
   * - write any return value back to preserved usr mode registers.
   */

  switch (id){
    case 0x00: { // 0x00 => yield()
        schedule(ctx);

        break;
      }

    case 0x01: { // 0x01 => write( fd, x, n )
      int fd = (int)(ctx->gpr[0]);
      char *x = (char *)(ctx->gpr[1]);
      int n = (int)(ctx->gpr[2]);

      for (int i = 0; i < n; i++){
        PL011_putc(UART0, *x++, true);
      }

      ctx->gpr[0] = n;

      break;
    }

    case 0x03: { // SYS_FORK
      int pid_c = get_freepcb() + 1;
      ctx->gpr[0] = pid_c;

      memcpy(&pcb[pid_c - 1].ctx, ctx, sizeof(ctx_t));
      pcb[pid_c - 1].pid = pid_c;
      pcb[pid_c - 1].status = STATUS_CREATED;

      memcpy((void *)get_memloc(pid_c) - 0x00001000, (void *)get_memloc(current->pid) - 0x00001000, 0x00001000);
      uint32_t offset = (uint32_t)get_memloc(current->pid) - (uint32_t)pcb[current->pid - 1].ctx.sp;
      pcb[pid_c - 1].ctx.sp = ((uint32_t)get_memloc(pid_c)) - offset;
      pcb[pid_c - 1].ctx.gpr[0] = 0;
      pcb[pid_c - 1].status = STATUS_READY;

      n++;
      break;
    }

    case 0x04: { // SYS_EXIT
      pcb[current->pid - 1].status= STATUS_TERMINATED;
      pcb[current->pid - 1].pid = -1;
      schedule(ctx);

      break;
    }

    case 0x05: { //SYS_EXEC
      ctx->pc = (uint32_t)(ctx->gpr[0]);
      
      break;
    }

    case 0x06: { //SYS_KILL
      pcb[ctx->gpr[0] - 1].status = STATUS_TERMINATED;
      pcb[ctx->gpr[0] - 1].pid = -1;
      schedule(ctx);
      break;
    }

    case 0x07: { //SYS_NICE

      break;
    }


    case 0x08: { //SYS_MUTX
      int n    = ctx->gpr[0];
      int mode = ctx->gpr[1];
      int data = ctx->gpr[2];
      int r1   = 0; //RESULT

      if(mode == 1){ // MUTX_READ, AUTO-LOCK
        if(mutexes[n]->status == MUTX_LOCKED){
          r1 = MUTX_READ_FAIL;
        }
        else if(mutexes[n]->status == MUTX_UNLOCKED){
          mutexes[n]->status = MUTX_LOCKED;
          mutexes[n]->owner  = current->pid;
          r1 = mutexes[n]->data;
        }
        else{
          r1 = MUTX_ERROR;
        }
      }

      if(mode == 2){ //MUTX_WRITE, AUTO-LOCK, NO AUTO-UNLOCK
        if(mutexes[n]->status == MUTX_LOCKED){
          if(mutexes[n]->owner == current->pid){
            mutexes[n]->data = data;
            r1 = MUTX_WRITE_OK;
          } else{
            r1 = MUTX_WRITE_FAIL;
          }
        }
        else if(mutexes[n]->status == MUTX_UNLOCKED){
          mutexes[n]->status = MUTX_LOCKED;
          mutexes[n]->owner = current->pid;
          mutexes[n]->data = data;
          r1 = MUTX_WRITE_OK;
        }
        else{
          r1 = MUTX_WRITE_FAIL;
        }
      }

      if(mode == 3){ //MUTX_RELEASE
        if(mutexes[n]->status == MUTX_LOCKED){
          if(mutexes[n]->owner == current->pid){
            mutexes[n]->owner = 0;
            mutexes[n]->status = MUTX_UNLOCKED;
            r1 = MUTX_RELEASE_OK;
          }
        }
        else if(mutexes[n]->status == MUTX_UNLOCKED){
          r1 = MUTX_RELEASE_FAIL;
        }
        else{
          r1 = MUTX_ERROR;
        }
      }

      ctx->gpr[1] = r1;
      break;
    }

    case 0x09: { //SYS_PIDD
      int r = current->pid;
      ctx->gpr[5] = r;
      break;
      schedule(ctx);
    }

    default: { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
