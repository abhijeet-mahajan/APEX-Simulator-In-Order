/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DATA_FORWARDING 1

int ENABLE_DEBUG_MESSAGES=1;

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename,const char* function_code,const char* function_cycles)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */


  cpu->pc = 4000;
  cpu->function_cycles=atoi(function_cycles);

  if(strcmp(function_code , "simulate")==0){
    ENABLE_DEBUG_MESSAGES=0;
  }

  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_forwarding, 0, sizeof(int) * 32);


  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if(strcmp(stage->opcode,"ADD")==0){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  else if(strcmp(stage->opcode,"SUB")==0){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  else if(strcmp(stage->opcode,"LOAD")==0){
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
  else if (strcmp(stage->opcode, "STORE") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }
  else if(strcmp(stage->opcode,"MUL")==0){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  else if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  else if(strcmp(stage->opcode,"AND")==0){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  else if(strcmp(stage->opcode,"OR")==0){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  else if(strcmp(stage->opcode,"EX-OR")==0){
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);
  }
  else if(strcmp(stage->opcode,"NOP")==0){
    printf("%s ", stage->opcode);
  }
  else if(strcmp(stage->opcode,"BZ")==0){
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
  else if(strcmp(stage->opcode,"BNZ")==0){
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
  else if(strcmp(stage->opcode,"JUMP")==0){
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
  }
  else if(strcmp(stage->opcode,"HALT")==0){
    printf("%s", stage->opcode);
  }




}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];


  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];

    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/

    if(cpu->stage[DRF].stalled) {
      stage->stalled=1;
    } else{
      cpu->stage[DRF] = cpu->stage[F];
    }

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
  else if(stage->stalled){
    if(!cpu->stage[DRF].stalled){
      stage->stalled=0;
      cpu->stage[DRF] = cpu->stage[F];
    }
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }

  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage *stage = &cpu->stage[DRF];

  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */

    if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0 ||
        strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 ||
        strcmp(stage->opcode, "EX-OR") == 0) {

      /* Check if Sources are valid. If not check if forwrding possible. if not stall.*/
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2]) {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
      } else if(ENABLE_DATA_FORWARDING){
        //check if load is in the ex. Wait for it to go to memory in that case.
        if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs1 || cpu->stage[EX].rd == stage->rs2)){
          stage->stalled=1;
        } else {

          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];

          if (!cpu->regs_valid[stage->rs1]) {
            stage->rs1_value = cpu->regs_forwarding[stage->rs1];
          }
          if (!cpu->regs_valid[stage->rs2]) {
            stage->rs2_value = cpu->regs_forwarding[stage->rs2];
          }
          cpu->regs_valid[stage->rd] = 0;
        }
      } else{
        stage->stalled=1;
      }
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      if (cpu->regs_valid[stage->rs1]) {
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->regs_valid[stage->rd] = 0;
      } else if(ENABLE_DATA_FORWARDING){
        //check if load is in the ex. Wait for it to go to memory in that case.
        if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs1)){
          stage->stalled=1;
        } else {
          stage->rs1_value = cpu->regs[stage->rs1];

          if (!cpu->regs_valid[stage->rs1]) {
            stage->rs1_value = cpu->regs_forwarding[stage->rs1];
          }
          cpu->regs_valid[stage->rd] = 0;
        }
      } else {
        stage->stalled = 1;
      }
    } else if (strcmp(stage->opcode, "STORE") == 0) {
      if (cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2]) {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
      }  else if(ENABLE_DATA_FORWARDING){
        //check if load is in the ex and the second source(address) of store is same as destination of load. stall in that case.
        if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs2)){
          stage->stalled=1;
        } else {
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];

          if (!cpu->regs_valid[stage->rs1]) {
            stage->rs1_value = cpu->regs_forwarding[stage->rs1];
          }
          if (!cpu->regs_valid[stage->rs2]) {
            stage->rs2_value = cpu->regs_forwarding[stage->rs2];
          }
        }
      } else {
        stage->stalled = 1;
      }
    }
      /* No Register file read needed for MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs_valid[stage->rd] = 0;
    } else if (strcmp(stage->opcode, "BZ") == 0) {
      if(!ENABLE_DATA_FORWARDING){
        stage->stalled = 1;
      }
    } else if (strcmp(stage->opcode, "BNZ") == 0) {
      if(!ENABLE_DATA_FORWARDING){
        stage->stalled = 1;
      }
    } else if (strcmp(stage->opcode, "JUMP") == 0) {
      if (cpu->regs_valid[stage->rs1]) {
        stage->rs1_value = cpu->regs[stage->rs1];
      }else if(ENABLE_DATA_FORWARDING){
        //check if load is in the ex
        if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs1)){
          stage->stalled=1;
        } else {
          stage->rs1_value = cpu->regs[stage->rs1];
          if (!cpu->regs_valid[stage->rs1]) {
            stage->rs1_value = cpu->regs_forwarding[stage->rs1];
          }
        }
      } else {
        stage->stalled = 1;
      }

    } else if (strcmp(stage->opcode, "HALT") == 0) {
    } else if (strcmp(stage->opcode, "NOP") == 0) {
    } else if (strcmp(stage->opcode, "") == 0) {

    }

    /* Copy data from decode latch to execute latch*/

    if (cpu->stage[EX].stalled) {
      stage->stalled = 1;
    } else if (stage->stalled) {
      CPU_Stage nop;
      Create_NOP(cpu, &nop);
      cpu->stage[EX] = nop;
    } else {
      cpu->stage[EX] = cpu->stage[DRF];
    }
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  } else if (stage->stalled) {
    if (!cpu->stage[EX].stalled) {
      if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 ||
          strcmp(stage->opcode, "MUL") == 0 || strcmp(stage->opcode, "AND") == 0 ||
          strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0) {
        /* Check if Sources are valid. If not check if forwrding possible. if not stall.*/
        if (cpu->regs_valid[cpu->stage[DRF].rs1] && cpu->regs_valid[cpu->stage[DRF].rs2]) {
          stage->stalled = 0;
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          cpu->regs_valid[stage->rd] = 0;
          cpu->stage[EX] = cpu->stage[DRF];
        }else if(ENABLE_DATA_FORWARDING){

          stage->stalled = 0;
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          if(!cpu->regs_valid[stage->rs1]){
            stage->rs1_value=cpu->regs_forwarding[stage->rs1];
          }
          if(!cpu->regs_valid[stage->rs2]){
            stage->rs2_value=cpu->regs_forwarding[stage->rs2];
          }
          cpu->regs_valid[stage->rd] = 0;
          cpu->stage[EX] = cpu->stage[DRF];
        }else {
          CPU_Stage nop;
          Create_NOP(cpu, &nop);
          cpu->stage[EX] = nop;
        }
      } else if (strcmp(stage->opcode, "LOAD") == 0) {
        /* Check if Source is valid. If not check if forwrding possible. if not stall.*/
        if (cpu->regs_valid[cpu->stage[DRF].rs1]) {
          stage->stalled = 0;
          cpu->regs_valid[stage->rd] = 0;
          stage->rs1_value = cpu->regs[stage->rs1];
          cpu->stage[EX] = cpu->stage[DRF];
        }else if(ENABLE_DATA_FORWARDING){
          //check if load is in the ex. Wait for it to go to memory in that case.
          if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs1)){
            stage->stalled=1;
            CPU_Stage nop;
            Create_NOP(cpu, &nop);
            cpu->stage[EX] = nop;
          } else {
            /*Fetch the source from forwarding logic*/
            stage->stalled = 0;
            stage->rs1_value = cpu->regs[stage->rs1];

            if (!cpu->regs_valid[stage->rs1]) {
              stage->rs1_value = cpu->regs_forwarding[stage->rs1];
            }
            cpu->regs_valid[stage->rd] = 0;
            cpu->stage[EX] = cpu->stage[DRF];
          }
        }else {
          CPU_Stage nop;
          Create_NOP(cpu, &nop);
          cpu->stage[EX] = nop;
        }
      } else if (strcmp(stage->opcode, "STORE") == 0) {
        /* Check if Sources are valid. If not check if forwrding possible. if not stall.*/
        if (cpu->regs_valid[cpu->stage[DRF].rs1] && cpu->regs_valid[cpu->stage[DRF].rs2]) {
          stage->stalled = 0;
          stage->rs1_value = cpu->regs[stage->rs1];
          stage->rs2_value = cpu->regs[stage->rs2];
          cpu->stage[EX] = cpu->stage[DRF];
        }else if(ENABLE_DATA_FORWARDING){
          //check if load is in the ex and the second source(address) of store is same as destination of load. stall in that case.
          if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs2)){
            stage->stalled=1;
            CPU_Stage nop;
            Create_NOP(cpu, &nop);
            cpu->stage[EX] = nop;
          } else {
            /*Fetch the sources from forwarding logic*/
            stage->stalled = 0;
            stage->rs1_value = cpu->regs[stage->rs1];
            stage->rs2_value = cpu->regs[stage->rs2];

            if (!cpu->regs_valid[stage->rs1]) {
              stage->rs1_value = cpu->regs_forwarding[stage->rs1];
            }
            if (!cpu->regs_valid[stage->rs2]) {
              stage->rs2_value = cpu->regs_forwarding[stage->rs2];
            }
            cpu->stage[EX] = cpu->stage[DRF];
          }
        }else {
          CPU_Stage nop;
          Create_NOP(cpu, &nop);
          cpu->stage[EX] = nop;
        }
      } else if (strcmp(stage->opcode, "MOVC") == 0) {
        /*No Source. No overhead*/
        stage->stalled = 0;
        cpu->regs_valid[stage->rd] = 0;
        cpu->stage[EX] = cpu->stage[DRF];
      } else if (strcmp(stage->opcode, "BZ") == 0) {
        /* Check for nearest Arithmetic instruction. This will ignore any other instructions. e.g And,Load*/
        if (strcmp(cpu->stage[EX].opcode, "ADD") == 0 || strcmp(cpu->stage[EX].opcode, "SUB") == 0 ||
            strcmp(cpu->stage[EX].opcode, "MUL") == 0 || strcmp(cpu->stage[MEM].opcode, "ADD") == 0 ||
            strcmp(cpu->stage[MEM].opcode, "SUB") == 0 || strcmp(cpu->stage[MEM].opcode, "MUL") == 0 ||
            strcmp(cpu->stage[WB].opcode, "ADD") == 0 || strcmp(cpu->stage[WB].opcode, "SUB") == 0 ||
            strcmp(cpu->stage[WB].opcode, "MUL") == 0)
        {
          stage->stalled = 1;
          CPU_Stage nop;
          Create_NOP(cpu, &nop);
          cpu->stage[EX] = nop;
        } else {
          stage->stalled = 0;
          cpu->stage[EX] = cpu->stage[DRF];
        }
      } else if (strcmp(stage->opcode, "BNZ") == 0) {
        /* Check for nearest Arithmetic instruction. This will ignore any other instructions. e.g And,Load*/
        if (strcmp(cpu->stage[EX].opcode, "ADD") == 0 || strcmp(cpu->stage[EX].opcode, "SUB") == 0 ||
            strcmp(cpu->stage[EX].opcode, "MUL") == 0 || strcmp(cpu->stage[MEM].opcode, "ADD") == 0 ||
            strcmp(cpu->stage[MEM].opcode, "SUB") == 0 || strcmp(cpu->stage[MEM].opcode, "MUL") == 0 ||
            strcmp(cpu->stage[WB].opcode, "ADD") == 0 || strcmp(cpu->stage[WB].opcode, "SUB") == 0 ||
            strcmp(cpu->stage[WB].opcode, "MUL") == 0) {
          stage->stalled = 1;
          CPU_Stage nop;
          Create_NOP(cpu, &nop);
          cpu->stage[EX] = nop;
        } else {
          stage->stalled = 0;
          cpu->stage[EX] = cpu->stage[DRF];
        }
      } else if (strcmp(stage->opcode, "JUMP") == 0) {
        if (cpu->regs_valid[stage->rs1]) {
          stage->stalled = 0;
          stage->rs1_value = cpu->regs[stage->rs1];
          cpu->stage[EX] = cpu->stage[DRF];

        } else if(ENABLE_DATA_FORWARDING){
          /*check if load is in the ex and has a dependency on source. Stall in that case. Else read the value for forwarding mechanism*/
          if(strcmp(cpu->stage[EX].opcode,"LOAD")==0 && ( cpu->stage[EX].rd == stage->rs1)){
            stage->stalled = 1;
            CPU_Stage nop;
            Create_NOP(cpu, &nop);
            cpu->stage[EX] = nop;
          } else {
            stage->stalled = 0;
            stage->rs1_value = cpu->regs[stage->rs1];
            if (!cpu->regs_valid[stage->rs1]) {
              stage->rs1_value = cpu->regs_forwarding[stage->rs1];
            }
            cpu->stage[EX] = cpu->stage[DRF];
          }
        } else {
          stage->stalled = 1;
          CPU_Stage nop;
          Create_NOP(cpu, &nop);
          cpu->stage[EX] = nop;

        }
      } else if (strcmp(stage->opcode, "HALT") == 0) {
        stage->stalled = 0;
        cpu->stage[EX] = cpu->stage[DRF];
      }else if (strcmp(stage->opcode, "") == 0) {
        stage->stalled = 0;
        cpu->stage[EX] = cpu->stage[DRF];
      }

    } else{}
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }

  }
  return 0;

}

/*
 *  Execute Stage of APEX Pipeline
 */
int
execute(APEX_CPU* cpu)
{

  CPU_Stage* stage = &cpu->stage[EX];
  if (!stage->busy && !stage->stalled) {
    /* Calculate. Update zero flag. Update forwarding value*/
    if (strcmp(stage->opcode, "ADD") == 0) {
      stage->buffer=stage->rs1_value+stage->rs2_value;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
        cpu->zero_flag = stage->buffer == 0 ? 1 : 0;
      }
    }
      /* Calculate. Update zero flag. Update forwarding value*/
    else if (strcmp(stage->opcode, "SUB") == 0) {
      stage->buffer=stage->rs1_value-stage->rs2_value;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
        cpu->zero_flag = stage->buffer == 0 ? 1 : 0;
      }
    }
    else if(strcmp(stage->opcode,"LOAD")==0){
      stage->mem_address=stage->rs1_value + stage->imm;
    }
      /* Store. Check if next instruction is load and if it's source 1(value to be added) is potential forwarding candidate */
    else if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address=stage->rs2_value+stage->imm;
      if(ENABLE_DATA_FORWARDING){
        if(strcmp(cpu->stage[MEM].opcode,"LOAD")==0 && cpu->stage[MEM].rd == stage->rs1){
          stage->rs1_value=cpu->regs_forwarding[stage->rs1];
        }
      }
    }
      /* Calculate. Stall for this cycle. Will take care of it in next.*/
    else if(strcmp(stage->opcode,"MUL")==0){
      stage->buffer=stage->rs1_value*stage->rs2_value;
      stage->stalled=1;

    }
      /* Calculate. Update forwarding value*/
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer=stage->imm + 0;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }

    }
      /* Calculate. Update forwarding value*/
    else if (strcmp(stage->opcode, "AND") == 0) {
      stage->buffer=stage->rs1_value & stage->rs2_value;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }

    }
      /* Calculate. Update forwarding value*/
    else if (strcmp(stage->opcode, "OR") == 0) {
      stage->buffer=stage->rs1_value | stage->rs2_value;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Calculate. Update forwarding value*/
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      stage->buffer=stage->rs1_value ^ stage->rs2_value;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }

    else if(strcmp(stage->opcode,"NOP")==0){
    }
    else if(strcmp(stage->opcode,"BZ")==0){
    }
    else if(strcmp(stage->opcode,"BNZ")==0){
    }
    else if(strcmp(stage->opcode,"JUMP")==0){
      stage->buffer=stage->rs1_value+stage->imm;
    }
    /*Make all previous instructions nop.*/
    else if(strcmp(stage->opcode,"HALT")==0){
      CPU_Stage nopStage;
      Create_NOP(cpu,&nopStage);
      cpu->stage[DRF] = nopStage;
      cpu->stage[F] = nopStage;
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }


    /* Copy data from Execute latch to Memory latch if the stage isn't stalled. Else add Nop*/
    if(!stage->stalled) {
      cpu->stage[MEM] = cpu->stage[EX];
    } else {
      CPU_Stage nopStage;
      Create_NOP(cpu,&nopStage);
      cpu->stage[MEM] = nopStage;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute", stage);
    }

  }
  else if(stage->stalled){
    /* Will only come here in case of Multiplication instruction. Update forwarding value. Update Zero flag*/
    stage->stalled=0;
    if(ENABLE_DATA_FORWARDING){
      cpu->regs_forwarding[stage->rd]=stage->buffer;
      cpu->zero_flag = stage->buffer == 0 ? 1 : 0;
    }

    cpu->stage[MEM] = cpu->stage[EX];
//    cpu->stage[EX] = cpu->stage[DRF];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute", stage);
    }

  }

  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 */
int
memory(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {

    /* Update forwarding value.*/
    if (strcmp(stage->opcode, "ADD") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update forwarding value.*/
    else if (strcmp(stage->opcode, "SUB") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Load the data in buffer from memory. Update forwarding value.*/
    else if(strcmp(stage->opcode,"LOAD")==0){
      stage->buffer=cpu->data_memory[stage->mem_address];

      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }

    }
      /* Store. Insert Source 1 into the Memory. */
    else if (strcmp(stage->opcode, "STORE") == 0) {
      cpu->data_memory[stage->mem_address]=stage->rs1_value;
    }
      /* Update forwarding value.*/
    else if (strcmp(stage->opcode, "MUL") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update forwarding value.*/
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update forwarding value.*/
    else if (strcmp(stage->opcode, "AND") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update forwarding value.*/
    else if (strcmp(stage->opcode, "OR") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update forwarding value.*/
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
    else if(strcmp(stage->opcode,"NOP")==0){
    }
    else if(strcmp(stage->opcode,"BZ")==0){
      if(cpu->zero_flag){
        //Branch taken when zero-flag is reset.
        cpu->pc=stage->pc + stage->imm;
        CPU_Stage nopStage;
        Create_NOP(cpu,&nopStage);
        cpu->stage[DRF] = nopStage;
        cpu->regs_valid[cpu->stage[EX].rd]=1;
        cpu->stage[EX]=nopStage;
      } else{
        //Branch Not Taken. Continue with execution;
      }
    }
    else if(strcmp(stage->opcode,"BNZ")==0){
      if(!cpu->zero_flag){
        //branch taken when zero-flag is set.
        cpu->pc=stage->pc + stage->imm;
        CPU_Stage nopStage;
        Create_NOP(cpu,&nopStage);
        cpu->stage[DRF] = nopStage;
        cpu->regs_valid[cpu->stage[EX].rd]=1;
        cpu->stage[EX]=nopStage;
      } else{
        //Branch Not Taken. Continue with execution;
      }
    }
      /* Free up destinations of DRF and EX. Set counter to new PC. Insert nop in previous 2.*/
    else if(strcmp(stage->opcode,"JUMP")==0){
      cpu->pc=stage->buffer;

      cpu->regs_valid[cpu->stage[DRF].rd]=1;
      cpu->regs_valid[cpu->stage[EX].rd]=1;

      CPU_Stage nopStage;
      Create_NOP(cpu,&nopStage);
      cpu->stage[DRF] = nopStage;
      cpu->stage[F] = nopStage;
      cpu->stage[EX] = nopStage;
    }
      /*Insert NOP in previous stage and stall it.*/
    else if(strcmp(stage->opcode,"HALT")==0){
      CPU_Stage nopStage;
      Create_NOP(cpu,&nopStage);
      cpu->stage[EX] = nopStage;
      cpu->stage[EX].stalled=1;
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory", stage);
    }
  } else if(stage->stalled){
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory", stage);
    }
  }
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {
    /* Update Destination Register, Make it valid and check for forwarding. Update Zero Flag*/
    if(strcmp(stage->opcode,"ADD") == 0 || strcmp(stage->opcode,"SUB") == 0 || strcmp(stage->opcode,"MUL") == 0)
    {
      cpu->regs[stage->rd]=stage->buffer;
      cpu->regs_valid[stage->rd]=1;
      cpu->zero_flag = stage->buffer == 0 ? 1 : 0;

      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update Destination Register, Make it valid and check for forwarding.*/
    else if(strcmp(stage->opcode,"AND") == 0 || strcmp(stage->opcode,"OR") == 0 || strcmp(stage->opcode,"EX-OR") == 0 ){
      cpu->regs[stage->rd]=stage->buffer;
      cpu->regs_valid[stage->rd]=1;

      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      /* Update Destination Register, Make it valid and check for forwarding.*/
    else if(strcmp(stage->opcode,"LOAD")==0){
      cpu->regs[stage->rd]=stage->buffer;
      cpu->regs_valid[stage->rd]=1;

      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
      //STORE
    else if(strcmp(stage->opcode,"STORE")==0){
      //no action
    }
      /* Update Destination Register, Make it valid and check for forwarding.*/
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=1;
      if(ENABLE_DATA_FORWARDING){
        cpu->regs_forwarding[stage->rd]=stage->buffer;
      }
    }
    else if(strcmp(stage->opcode,"NOP")==0){
    }
      /* if instruction is halt PC will be set to a MAX value which won't be in the code*/
    else if(strcmp(stage->opcode,"HALT")==0){
      cpu->pc=cpu->pc+12000;
      CPU_Stage nopStage;
      Create_NOP(cpu,&nopStage);
      cpu->stage[MEM] = nopStage;
      cpu->stage[MEM].stalled=1;
    }
    else if(strcmp(stage->opcode,"JUMP")==0){
    }

    if(strcmp(stage->opcode,"NOP")!=0)
      cpu->ins_completed++;

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  int initial_PC_Value=cpu->pc;
  while (1) {
    /* All the instructions committed, so exit */
    /*Initial Completion logic*/
//    if (cpu->ins_completed == cpu->code_memory_size) {
//      printf("(apex) >> Simulation Complete- %d Instructions \n",(cpu->ins_completed));
//      Print_regs_content(cpu);//this will print the data of all the regs
//      break;
//    }

    /* All the instructions committed, so exit */
    /*New Completion logic*/
    if(cpu->pc >= ((cpu->code_memory_size * 4)+initial_PC_Value +16) || cpu->clock ==cpu->function_cycles ){

      if (ENABLE_DEBUG_MESSAGES) {
        printf("--------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock);
        printf("--------------------------------\n");
      }

      printf("(apex) >> Simulation Complete \n");
      printf("Total Instructions Present: %d, Total instructions processed: %d \n",cpu->code_memory_size,cpu->ins_completed);
      printf("Total clock cycles taken: %d \n",cpu->clock);
      Print_regs_content(cpu);//this will print the data of all the regs
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu);
    memory(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
  }

  return 0;
}

/*Output printing function*/
void Print_regs_content(APEX_CPU* cpu){
  printf("\n\n=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n\n");
  char isValid[8] = "VALID  ";
  for(int i=0;i<16;i++){
    if(cpu->regs_valid[i] == 0){
      strcpy(isValid,"INVALID");
    } else{
      strcpy(isValid,"VALID  ");
    }
    printf("|\tREG[%d]\t|\tValue = %d\t|\tStatus = %s\t|\n",i,cpu->regs[i],isValid);
  }

  printf("\n\n============== STATE OF DATA MEMORY =============\n\n");
  for(int j=0;j<100;j++){
    printf("|\tMEM[%d]\t|\tData Value = %d\t|\n",j,cpu->data_memory[j]);
  }
}

/* function to create a NOP with default values*/
void Create_NOP(APEX_CPU* cpu,CPU_Stage* nopStage){
  strcpy(nopStage->opcode,"NOP");
  nopStage->busy=0;
  nopStage->stalled=0;
  nopStage->pc=cpu->stage[EX].pc;
}

