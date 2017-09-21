#include "schedulersim.hpp"
#include <cstdio>
#include <math.h>
#include <stdio.h>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <iostream>  
#include <bitset>
#include <tuple>
#include <algorithm>

using namespace std;

int current_cycle = 1;

/**
 * Subroutine that returns how long it takes to complete a specific operation type
 *
 * @return                      Instruction latency in cycles
 */


int get_inst_latency(int op) {
    if(op == 0) {
        return 2;
    } else if(op == 1) {
        return 15;
    } else if(op == 2) {
        return 20;
    }
    std::printf("\nInvalid OP type:%d\n", op);
    std::exit(1);
}

/**
 * XXX: You are welcome to define and set any global classes and variables as needed.
 */
 
 
 //Global variables
 int i=0, k=0;
 char rs_type = RSTYPE_PER_FU;
 int rs_rows = 0;
 
 int rs_rows_Add = 0;
 int rs_rows_Div = 0;
 int rs_rows_Mem = 0;
 
 int rs_cols = 0;
 int num_reg = 0;
 unsigned int issue_counter = 0;
 unsigned int execute_counter = 0;
 unsigned int complete_counter = 0;
 
 std::uint64_t n_insts=0;
 std::uint64_t n_cycles=0;
 double inst_pc=0;
 std::uint64_t n_issue_stall=0;
 std::uint32_t n_max_fired=0;
 std::uint32_t n_fired=0;
 std::uint32_t n_max_completed=0;
 std::uint32_t n_completed=0;
 std::uint32_t n_max_active[NUM_OPS]={0,0,0};
 
 int oldest_ADD_issued = 0;
 int oldest_ADD_issued_index = 0;
 int oldest_DIV_issued = 0;
 int oldest_DIV_issued_index = 0;
 int oldest_MEM_issued = 0;
 int oldest_MEM_issued_index = 0;
 

 //Vectors for the FU RS
 
 std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > rsAdd;
 std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > rsDiv;
 std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > rsMem;
 
 //Vector for the Unified RS
 std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > rsUnif;
 
 //Vector for the RAT
 std::vector<int> rat;
 
 //Global Tuple variables for the RS vectors 
 std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> temp_row;
 std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> temp_row_2;
 
 //Method for converting the Source Data based on whether source is Immediate, from an ARF, or an Alias from an RS
 int sourceDataConversion(int);


/**
 * Subroutine for initializing the scheduler (unified reservation station type).
 * You may in,italize any global or heap variables as needed.
 * XXX You're responsible for completing this routine.
 *
 * @param[in]   num_registers   The number of registers in the instructions
 * @param[in]   rs_size         The number of entries for the unified RS
 */
void scheduler_unified_init(int num_registers, int rs_size) {
	
	rs_type = RSTYPE_UNIFIED;
	
	rs_rows = rs_size;
	
	rs_cols = rsUnif.size();
	
	num_reg = num_registers;
	
	rat.assign (num_reg,0); //Initialize the RAT with 0s
	
	//printf("\nRAT: ");

	for (i=0; i < num_reg; i++) {
		//printf("%d,",rat.at(i));
		
	}
	
	temp_row = std::make_tuple((-1*(i+1)),OP_INVALID,0,0,0,0,0,0,1,0,0,0,0,0); //Initialize the RS with default values
		
	for(i=0; i<rs_size; i++) {
		
		rsUnif.push_back (std::make_tuple((-1*(i+1)),OP_INVALID,0,0,0,0,0,0,1,0,0,0,0,0));
		
	}
	
	//printf("\nVector of Unified RS = ");
	
	
	for ( const auto& i : rsUnif ) {
	//cout << get<0>(i) << get<1>(i) << get<2>(i) << get<3>(i) << get<4>(i) << get<5>(i) << get<6>(i) << get<7>(i) << get<8>(i) << get<9>(i) << get<10>(i) <<get<11>(i) <<get<12>(i) <<get<13>(i) << endl;
	}
	
	//printf("\n\n");
}

/**
 * Subroutine for initializing the scheduler (per-functional unit reservation station type).
 * You may initalize any global or heap variables as needed.
 * XXX You're responsible for completing this routine.
 *
 * @param[in]   num_registers   The number of registers in the instructions
 * @param[in]   rs_sizes        An array of size 3 that contains the number of entries for each
 *                              op_type
 *                              rs_sizes = [4,2,1] means 4 ADD RS, 2 DIV RS, 1 MEM RS
 */
void scheduler_per_fu_init(int num_registers, int rs_sizes[]) {
	
	rs_type = RSTYPE_PER_FU;
	
	rs_rows_Add = rs_sizes[0];
	
	rs_rows_Div = rs_sizes[1];
	
	rs_rows_Mem = rs_sizes[2];
	
	rs_cols = rsAdd.size();
	
	num_reg = num_registers;
	
	rat.assign (num_reg,0);
	
	//printf("\nRAT: ");

	for (i=0; i < num_reg; i++) {
		//printf("%d,",rat.at(i));
		
	}
	
	temp_row = std::make_tuple((-1*(i+1)),OP_INVALID,0,0,0,0,0,0,1,0,0,0,0,0);
	
	//Initialize the ADD RS with default values
	for(i=0; i<rs_sizes[0]; i++) {
		
		rsAdd.push_back (std::make_tuple((-1*(i+1)),OP_INVALID,0,0,0,0,0,0,1,0,0,0,0,0));
		
	}
	
	//Initialize the DIV RS with default values
	for(i=rs_sizes[0]; i<(rs_sizes[0] + rs_sizes[1]) ; i++) {
		
		rsDiv.push_back (std::make_tuple((-1*(i+1)),OP_INVALID,0,0,0,0,0,0,1,0,0,0,0,0));
		
	}
	
	//Initialize the MEM RS with default values
	for(i=(rs_sizes[0] + rs_sizes[1]); i<(rs_sizes[0] + rs_sizes[1] + rs_sizes[2]); i++) {
		
		rsMem.push_back (std::make_tuple((-1*(i+1)),OP_INVALID,0,0,0,0,0,0,1,0,0,0,0,0));
		
	}
	
	//printf("\nVector of ADD RS = ");
		
	for ( const auto& i : rsAdd ) {
	//cout << get<0>(i) << get<1>(i) << get<2>(i) << get<3>(i) << get<4>(i) << get<5>(i) << get<6>(i) << get<7>(i) << get<8>(i) << get<9>(i) << get<10>(i) <<get<11>(i) <<get<12>(i) <<get<13>(i) << endl;
	}
	
	//printf("\n");
	
	//printf("\nVector of DIV RS = ");
		
	for ( const auto& i : rsDiv ) {
	//cout << get<0>(i) << get<1>(i) << get<2>(i) << get<3>(i) << get<4>(i) << get<5>(i) << get<6>(i) << get<7>(i) << get<8>(i) << get<9>(i) << get<10>(i) <<get<11>(i) <<get<12>(i) <<get<13>(i) << endl;
	}
	
	//printf("\n");
	
	//printf("\nVector of MEM RS = ");
		
	for ( const auto& i : rsMem ) {
	//cout << get<0>(i) << get<1>(i) << get<2>(i) << get<3>(i) << get<4>(i) << get<5>(i) << get<6>(i) << get<7>(i) << get<8>(i) << get<9>(i) << get<10>(i) <<get<11>(i) <<get<12>(i) <<get<13>(i) << endl;
	}
	
	//printf("\n");
	
}

/**
 * Subroutine that tries to issue an instruction to the reservation station. You need to 
 * choose the appropriate RS depending on the RS type and op_type and update the RAT.
 * XXX You're responsible for completing this routine.
 *
 * @param[in]   op_type         The FU to use
 * @param[in]   dest            The destination register
 * @param[in]   src1            The first source register (-1 if unused)
 * @param[in]   src2            The seconde source register (-1 if unused)
 * @param[out]  p_stats         Pointer to the stats structure
 *
 * @return                      true if successful, false if we failed
 */

 //Method to write-back values 
 void write_back_in_RS(std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsUnif, int rs_rows, std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> &temp_row){
	
	for (k=0; k<rs_rows; k++){ // Scan the entire RS
		temp_row_2 = rsUnif.at(k); //temp = current row
		if(std::get<9>(temp_row_2) > std::get<9>(temp_row) && (std::get<4>(temp_row_2) == std::get<0>(temp_row) || std::get<6>(temp_row_2) == std::get<0>(temp_row))){ //If any instruction was issued after the currently completed instruction, and either of it's sources depend on the destination of the current instruction
			
			if(std::get<4>(temp_row_2) == std::get<0>(temp_row)){ //If source 1 depends on the current completed instruction
				std::get<4>(temp_row_2) = std::get<2>(temp_row); //Write-Back
				std::get<5>(temp_row_2) = 1;
			}
			if(std::get<6>(temp_row_2) == std::get<0>(temp_row)){ //If source 2 depends on the current completed instruction
				std::get<6>(temp_row_2) = std::get<2>(temp_row); //Write-Back
				std::get<7>(temp_row_2) = 1;
			}
			rsUnif.at(k) = temp_row_2; //Store the temp tuple back into the RS
		}
		
	}	
 }

 //Method for issuing an instruction
 bool issue(std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsUnif, int rs_rows, op_type op, int dest, int src1, int src2) {
	for (i=0; i<rs_rows; i++){ // Scan the entire RS
		temp_row = rsUnif.at(i);
		if(std::get<8>(temp_row) == 1){ //if empty=true, issue instruction
			issue_counter = current_cycle;
			//printf("\n\n");
			//printf("\nIssue cycle = %d",issue_counter);
			std::get<1>(temp_row) = op; //Instruction
			
			std::get<2>(temp_row) = dest; //Destination
			
			std::get<3>(temp_row) = 0; //Dest Ready
			
			
			std::get<4>(temp_row) = sourceDataConversion(src1); //Source1
			if(std::get<4>(temp_row) >= 0 ){ //Data ready condition 1
				std::get<5>(temp_row) = 1; //Source1 Ready
			}
			else{
				std::get<5>(temp_row) = 0; //Source1 Not Ready
			}
			
			std::get<6>(temp_row) = sourceDataConversion(src2); //Source2
			if(std::get<6>(temp_row) >= 0){ //Data ready condition 1
				std::get<7>(temp_row) = 1; //Source2 Ready
			}
			else{
				std::get<7>(temp_row) = 0; //Source2 Not Ready
			}
			
			std::get<8>(temp_row) = 0; //Empty Flag
			std::get<9>(temp_row) = issue_counter; //Issue Counter
			std::get<10>(temp_row) = 0; //Execution Counter
			std::get<11>(temp_row) = 0; //IsExecuting Flag
			std::get<12>(temp_row) = 0; //Complete Flag
			std::get<13>(temp_row) = 0; //Write-Back Counter
			
			rsUnif.at(i) = temp_row;
			
			rat.at(dest-1) = std::get<0>(temp_row);
			
			//printf("\nInstruction issued at cycle %d : %d %d %d %d", current_cycle, std::get<1>(temp_row), std::get<2>(temp_row), std::get<4>(temp_row), std::get<6>(temp_row));
			
		
		 
			//issue = true, raise flag
				
			//printf("\nRAT: ");

			for (i=0; i < num_reg; i++) {
				//printf("%d,",rat.at(i));
				
			}
			
			k=0;
			//printf("\nVector of Unified RS = ");

			for ( const auto& k : rsUnif ) {
			//cout << get<0>(k) << get<1>(k) << get<2>(k) << get<3>(k) << get<4>(k) << get<5>(k) << get<6>(k) << get<7>(k) << get<8>(k) << get<9>(k) << get<10>(k) <<get<11>(k) <<get<12>(k) <<get<13>(k) << endl;
			}
			
			n_insts++; //Increment the number of instructions issued counter
			
			return true;
			//break;
		}
	}
	n_issue_stall++;
	return false;
}

 //method to start execution of instructions
 void execute(std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsUnif, int rs_rows) {
	
	oldest_ADD_issued = -1;
	oldest_ADD_issued_index = -1;
	
	
	for (k=0; k<rs_rows; k++){ // Scan the entire RS
		temp_row = rsUnif.at(k);
		if(std::get<8>(temp_row) != 1 && std::get<11>(temp_row) != 1 && std::get<5>(temp_row)== 1 && std::get<7>(temp_row)== 1){ //IsEmpty = False & IsExecuting = False and both sources are ready
			
			if(oldest_ADD_issued == -1){ //If checking for the first time (default value)
				oldest_ADD_issued = std::get<9>(temp_row);
				oldest_ADD_issued_index = k;
			}
			else if((int)std::get<9>(temp_row)<oldest_ADD_issued){ //FInd out the oldest instruction (issued the oldest)
				oldest_ADD_issued = std::get<9>(temp_row);
				oldest_ADD_issued_index = k;
				
			}
			//printf("\noldest_ADD_issued_index = %d", oldest_ADD_issued_index);
			
			
			
		}
		
	}
	
	//Fire oldest instruction 
	for (i=0; i<rs_rows; i++){
		if( (i== oldest_ADD_issued_index) ){
			temp_row = rsUnif.at(i);
			if(std::get<8>(temp_row) != 1 && std::get<11>(temp_row) != 1 && std::get<5>(temp_row)== 1 && std::get<7>(temp_row)== 1){ //IsEmpty = False & IsExecuting = False & Source1 = Ready & Source2 = Ready
				if(std::get<5>(temp_row)== 1 && std::get<7>(temp_row)== 1){ //Both sources ready
					std::get<11>(temp_row) = 1; //IsExecuting = True
					std::get<10>(temp_row) = current_cycle; //Execution Counter = Current Cycle
					//printf("\nInstruction started at cycle %d : %d %d %d %d", current_cycle, std::get<1>(temp_row), std::get<2>(temp_row), std::get<4>(temp_row), std::get<6>(temp_row));
					rsUnif.at(i) = temp_row;
					
					n_fired++; //Increment the fired instruction counter
					
				
					
				}
			}
		}
		
	}
}

 //Find the instructions which should complete and update the flags
 void complete(std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsMem, int rs_rows_Mem,  std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsDiv, int rs_rows_Div,  std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsAdd, int rs_rows_Add) {
	for (i=0; i<rs_rows_Mem; i++){
		temp_row = rsMem.at(i);
		
		if(std::get<8>(temp_row) != 1 && std::get<11>(temp_row) == 1 && std::get<12>(temp_row) != 1){ //IsEmpty = False & IsExecuting = True & Complete = False
			if((current_cycle) == (int)(std::get<10>(temp_row) + get_inst_latency(std::get<1>(temp_row)))){ //If an instruction has taken its designated cycles
				
				//std::printf("\nOP type:%d\n", std::get<1>(temp_row));
				//std::printf("\nCurrent Cycle: %d\n", current_cycle);
				
				std::get<11>(temp_row) = 0; //IsExecuting = False
				std::get<12>(temp_row) = 1; //Complete = True
				std::get<13>(temp_row) = current_cycle+1; //Write-Back Cycle
				//std::printf("\nWrite-Back Cycle: %d\n", std::get<13>(temp_row));
				
				//printf("\nInstruction completed at cycle %d : %d %d %d %d", current_cycle, std::get<1>(temp_row), std::get<2>(temp_row), std::get<4>(temp_row), std::get<6>(temp_row));
				n_cycles = current_cycle;
				
				
				rsMem.at(i) = temp_row;
				n_completed++; //Increment the completed instruction counter
				complete_counter++;
			
			}
			
		}
		
		
		
		if(std::get<8>(temp_row) != 1 && std::get<12>(temp_row) == 1){ //IsEmpty = False & If Complete = True
			
			write_back_in_RS(rsMem, rs_rows_Mem, temp_row);
			write_back_in_RS(rsAdd, rs_rows_Add, temp_row);
			write_back_in_RS(rsDiv, rs_rows_Div, temp_row);
			
			//printf("\nOld RAT: ");
			for (k=0; k < num_reg; k++) {
				
				//printf("%d,",rat.at(k));
			
			}
			//Update RAT
			for (k=0; k < num_reg; k++) {	
				if(rat.at(k) == std::get<0>(temp_row)){
					rat.at(k) = 0;
				}
			}
			
			//printf("\nNew RAT: ");
			for (k=0; k < num_reg; k++) {
				
				//printf("%d,",rat.at(k));
			
			}
			std::get<8>(temp_row) = 1; //The completed instruction is cleared from RS, i.e. this RS entry is empty now
			
			//continue;
			rsMem.at(i) = temp_row;
			
			//printf("\nUpdated Vector of Unified RS = ");

			for ( const auto& k : rsMem ) {
			//cout << get<0>(k) << get<1>(k) << get<2>(k) << get<3>(k) << get<4>(k) << get<5>(k) << get<6>(k) << get<7>(k) << get<8>(k) << get<9>(k) << get<10>(k) <<get<11>(k) <<get<12>(k) <<get<13>(k) << endl;
			}
			
			//printf("\n\n");
			
		}
	}
}
 
 
 bool scheduler_try_issue(op_type op, int dest, int src1, int src2, scheduler_stats_t* p_stats) {
	switch(rs_type){
		
		case 'F': {
		
			switch(op){
				case (OP_INVALID):{break;}
				case (OP_ADD):{ //ADD Instruction
					return issue(rsAdd, rs_rows_Add, op, dest, src1, src2);
					break;
				}
				case (OP_DIV):{ //DIV Instruction
					return issue(rsDiv, rs_rows_Div, op, dest, src1, src2);
					break;
				}
				case (OP_MEM):{ //MEM Instruction
					return issue(rsMem, rs_rows_Mem, op, dest, src1, src2);
					break;
				}
				case (NUM_OPS):{break;}
				default:{break;}			
			}
			
			
			break;}
		
		case 'U': {
		///*
		
			
			for (i=0; i<rs_rows; i++){
				temp_row = rsUnif.at(i);
				if(std::get<8>(temp_row) == 1){ //if empty=true, issue instruction
					issue_counter = current_cycle;
					//printf("\nIssue cycle = %d",issue_counter);
					std::get<1>(temp_row) = op; //Instruction
					
					std::get<2>(temp_row) = dest; //Destination
					
					std::get<3>(temp_row) = 0; //Dest Ready
					
					
					std::get<4>(temp_row) = sourceDataConversion(src1); //Source1
					if(std::get<4>(temp_row) >= 0 ){ //Data ready condition 1
						std::get<5>(temp_row) = 1; //Source1 Ready
					}
					else{
						std::get<5>(temp_row) = 0; //Source1 Not Ready
					}
					
					std::get<6>(temp_row) = sourceDataConversion(src2); //Source2
					if(std::get<6>(temp_row) >= 0){ //Data ready condition 1
						std::get<7>(temp_row) = 1; //Source2 Ready
					}
					else{
						std::get<7>(temp_row) = 0; //Source2 Not Ready
					}
					
					std::get<8>(temp_row) = 0; //Empty Flag
					std::get<9>(temp_row) = issue_counter; //Issue Counter
					std::get<10>(temp_row) = 0; //Execution Counter
					std::get<11>(temp_row) = 0; //IsExecuting Flag
					std::get<12>(temp_row) = 0; //Complete Flag
					std::get<13>(temp_row) = 0; //Write-Back Counter
					
					rsUnif.at(i) = temp_row;
					
					rat.at(dest-1) = std::get<0>(temp_row);
					
					//printf("\nInstruction %d issued ", issue_counter);
					
				
				 
					//issue = true, raise flag
					
					//printf("\nRAT: ");

					for (i=0; i < num_reg; i++) {
						//printf("%d,",rat.at(i));
						
					}
					
					k=0;
					//printf("\nVector of Unified RS = ");

					for ( const auto& k : rsUnif ) {
					//cout << get<0>(k) << get<1>(k) << get<2>(k) << get<3>(k) << get<4>(k) << get<5>(k) << get<6>(k) << get<7>(k) << get<8>(k) << get<9>(k) << get<10>(k) <<get<11>(k) <<get<12>(k) <<get<13>(k) << endl;
					}
					
					
					
					n_insts++;
					
					return true;
					//break;
				}
			}
			n_issue_stall++;
		
			//*/
			break;
		}
	
		default: break;
		
	}
	
    return false;
}

/**
 * Subroutine that checks if all instructions have been drained from the pipeline
 *
 * @return                      true if no instructions are left
 */
bool scheduler_completed() {
	
	if(n_insts==complete_counter)
		return true;
	
    return false;
}

/**
 * Subroutine that increments the clock cycle and updates any system state
 *
 * @param[out]  p_stats         Pointer to the stats structure
 */
void calc_max_active(std::vector<std::tuple<int, op_type, int, bool, int, bool, int, bool, bool, unsigned int, unsigned int, bool, bool, unsigned int> > &rsUnif, int rs_rows){
	
	for (i=0; i<rs_rows; i++){
		
		temp_row = rsUnif.at(i);
		if(std::get<8>(temp_row) != 1 && std::get<11>(temp_row) == 1){ //IsEmpty = False && IsExecuting = True
			switch(std::get<1>(temp_row)){ //Switch type of instruction
				case (OP_INVALID):{break;}
				case (OP_ADD):{ //ADD Instruction
					n_max_active[0]++;
					break;
				}
				case (OP_DIV):{ //DIV Instruction
					n_max_active[1]++;
					break;
				}
				case (OP_MEM):{ //MEM Instruction
					n_max_active[2]++;
					break;
				}
				case (NUM_OPS):{break;}
				default:{break;}			
			}
		}
	}
	
	
	
 }

 void scheduler_step(scheduler_stats_t* p_stats) {
    current_cycle ++;
	//printf("\nCurrent cycle = %d", current_cycle);
    scheduler_clear_completed(p_stats);
	scheduler_start_ready(p_stats);
	
	n_max_active[0] = 0;
	n_max_active[1] = 0;
	n_max_active[2] = 0;
	switch(rs_type){ //Switch the RS type to calculate the maximum active instructions
		 
		case 'F': {
			
			calc_max_active(rsAdd, rs_rows_Add);
			calc_max_active(rsDiv, rs_rows_Div);
			calc_max_active(rsMem, rs_rows_Mem);
			
			break;
		}
		
		case 'U': {
			
			calc_max_active(rsUnif, rs_rows);
			
			break;
		}
		
		default: break;
	}
	p_stats-> max_active [0]= std::max(n_max_active[0], p_stats-> max_active [0]);
	p_stats-> max_active [1]= std::max(n_max_active[1], p_stats-> max_active [1]);
	p_stats-> max_active [2]= std::max(n_max_active[2], p_stats-> max_active [2]);
	
    
}

/**
 * Subroutine for firing (start executing) any ready instructions.
 * XXX You're responsible for completing this routine.
 *
 * @param[out]  p_stats         Pointer to the stats structure
 */
void scheduler_start_ready(scheduler_stats_t* p_stats) {
	
	n_max_fired = 0;
	n_fired = 0;
	
	switch(rs_type){
		
		case 'F': {
			
			execute(rsAdd, rs_rows_Add);

			execute(rsDiv, rs_rows_Div);
		
			execute(rsMem, rs_rows_Mem);

		break;}
		
		case 'U': {
			n_max_fired = 0;
			n_fired = 0;
			oldest_ADD_issued = -1;
			oldest_ADD_issued_index = -1;
			
			oldest_DIV_issued = -1;
			oldest_DIV_issued_index = -1;
			
			oldest_MEM_issued = -1;
			oldest_MEM_issued_index = -1;
			
			for (k=0; k<rs_rows; k++){
				temp_row = rsUnif.at(k);
				if(std::get<8>(temp_row) != 1 && std::get<11>(temp_row) != 1 && std::get<5>(temp_row)== 1 && std::get<7>(temp_row)== 1){ //IsEmpty = False & IsExecuting = False and both sources are ready
					if(std::get<1>(temp_row) == OP_ADD){ //Find out oldest ADD instruction
						if(oldest_ADD_issued == -1){
							oldest_ADD_issued = std::get<9>(temp_row);
							oldest_ADD_issued_index = k;
						}
						else if((int)std::get<9>(temp_row)<oldest_ADD_issued){
							oldest_ADD_issued = std::get<9>(temp_row);
							oldest_ADD_issued_index = k;
							
						}
						//printf("\noldest_ADD_issued_index = %d", oldest_ADD_issued_index);
					
					}
					if(std::get<1>(temp_row) == OP_DIV){ //Find out oldest DIV instruction	
						if(oldest_DIV_issued == -1){
							oldest_DIV_issued = std::get<9>(temp_row);
							oldest_DIV_issued_index = k;
						}
						else if((int)std::get<9>(temp_row)<oldest_DIV_issued){
							oldest_DIV_issued = std::get<9>(temp_row);
							oldest_DIV_issued_index = k;
							
						}
						//printf("\noldest_DIV_issued_index = %d", oldest_DIV_issued_index);
					
					}
					if(std::get<1>(temp_row) == OP_MEM){ //Find out oldest MEM instruction
						if(oldest_MEM_issued == -1){
							oldest_MEM_issued = std::get<9>(temp_row);
							oldest_MEM_issued_index = k;
						}
						else if((int)std::get<9>(temp_row)<oldest_MEM_issued){
							oldest_MEM_issued = std::get<9>(temp_row);
							oldest_MEM_issued_index = k;
							
						}
						//printf("\noldest_MEM_issued_index = %d", oldest_MEM_issued_index);
					
					}
				
				}
				
			}
			
			for (i=0; i<rs_rows; i++){
				if( (i== oldest_ADD_issued_index) || (i == oldest_DIV_issued_index)|| (i == oldest_MEM_issued_index)){
					temp_row = rsUnif.at(i);
					if(std::get<8>(temp_row) != 1 && std::get<11>(temp_row) != 1 && std::get<5>(temp_row)== 1 && std::get<7>(temp_row)== 1){ //IsEmpty = False & IsExecuting = False & Source1 = Ready & Source2 = Ready
						if(std::get<5>(temp_row)== 1 && std::get<7>(temp_row)== 1){ //Both sources ready
							std::get<11>(temp_row) = 1; //IsExecuting = True
							std::get<10>(temp_row) = current_cycle; //Execution Counter = Current Cycle
							//printf("\nInstruction started at cycle %d : %d %d %d %d", current_cycle, std::get<1>(temp_row), std::get<2>(temp_row), std::get<4>(temp_row), std::get<6>(temp_row));
							rsUnif.at(i) = temp_row;
							
							n_fired++;
							
						
							
						}
					}
				}
				
			}
			//*/
			
			
			
			
		break;}
		
		default: break;
	}
	
	n_max_fired = std::max(n_max_fired, n_fired); //Keep track of the max fired instruction value
	p_stats-> max_fired = std::max(n_max_fired, p_stats-> max_fired); //Update stats
	//printf("\nMax instructions fired in this cycle = %d", n_max_fired);
	
}

//Method to check the source type: Immediate, ARF, or Aliased from an RS entry
int sourceDataConversion(int src){
	
	if(src == -1){
		//printf("\nSource is immediate");
		return 0;
	}
	else{
		if(rat.at(src-1) == 0){
			//printf("\nSource is register %d", src);
			return src;
			
		}
		else{
			if(rat.at(src-1)<0){
			//printf("\nSource is alias at R%d = %d", src, rat.at(src-1));
			return (rat.at(src-1));
			}
			
		}
		
	}
	return 0;
}

/**
 * Subroutine for clearing any completed instructions.
 * XXX You're responsible for completing this routine.
 *
 * @param[out]  p_stats         Pointer to the stats structure
 */
void scheduler_clear_completed(scheduler_stats_t* p_stats) {
	///*
	n_max_completed = 0;
	n_completed = 0;
	
	switch(rs_type){
		
		case 'F': {
			complete(rsAdd, rs_rows_Add, rsDiv, rs_rows_Div, rsMem, rs_rows_Mem);

			complete(rsDiv, rs_rows_Div, rsMem, rs_rows_Mem, rsAdd, rs_rows_Add);
		
			complete(rsMem, rs_rows_Mem, rsAdd, rs_rows_Add, rsDiv, rs_rows_Div);
			
		break;}
		
		case 'U': {
			
			
			for (i=0; i<rs_rows; i++){
				temp_row = rsUnif.at(i);
				
				if(std::get<11>(temp_row) == 1 && std::get<12>(temp_row) != 1){ //IsExecuting = True & Complete = False
					if((current_cycle) == (int)(std::get<10>(temp_row) + get_inst_latency(std::get<1>(temp_row)))){ //If an instruction has taken its designated cycles
						
						//std::printf("\nOP type:%d\n", std::get<1>(temp_row));
						//std::printf("\nCurrent Cycle: %d\n", current_cycle);
						
						std::get<11>(temp_row) = 0; //IsExecuting = False
						std::get<12>(temp_row) = 1; //Complete = True
						std::get<13>(temp_row) = current_cycle+1; //Write-Back Cycle
						//std::printf("\nWrite-Back Cycle: %d\n", std::get<13>(temp_row));
						//printf("\nInstruction %d completed at cycle %d", std::get<9>(temp_row), current_cycle);
						n_cycles = current_cycle;
						
						
						rsUnif.at(i) = temp_row;
						n_completed++;
						complete_counter++;
					
					}
					
				}
				
				
				//When completed instructions found, update flags
				if(std::get<12>(temp_row) == 1){ //If Complete = True
					
					for (k=0; k<rs_rows; k++){ // Scan the entire RS
						temp_row_2 = rsUnif.at(k);
						if(std::get<9>(temp_row_2) > std::get<9>(temp_row) && (std::get<4>(temp_row_2) == std::get<0>(temp_row) || std::get<6>(temp_row_2) == std::get<0>(temp_row))){ //If any instruction was issued after the currently completed instruction, and either of it's sources depend on the destination of the current instruction
							
							if(std::get<4>(temp_row_2) == std::get<0>(temp_row)){ //If source 1 depends on the current completed instruction
								std::get<4>(temp_row_2) = std::get<2>(temp_row); //Write-Back
								std::get<5>(temp_row_2) = 1;
							}
							if(std::get<6>(temp_row_2) == std::get<0>(temp_row)){ //If source 2 depends on the current completed instruction
								std::get<6>(temp_row_2) = std::get<2>(temp_row); //Write-Back
								std::get<7>(temp_row_2) = 1;
							}
							rsUnif.at(k) = temp_row_2;
						}
						
					}
					
					//printf("\nOld RAT: ");
					for (k=0; k < num_reg; k++) {
						
						//printf("%d,",rat.at(k));
					
					}
					//Update RAT
					for (k=0; k < num_reg; k++) {	
						if(rat.at(k) == std::get<0>(temp_row)){
							rat.at(k) = 0;
						}
					}
					
					//printf("\nNew RAT: ");
					for (k=0; k < num_reg; k++) {
						
						//printf("%d,",rat.at(k));
					
					}
					std::get<8>(temp_row) = 1; //The completed instruction is cleared from RS, i.e. this RS entry is empty now
					
					//continue;
					rsUnif.at(i) = temp_row;
					
					//printf("\nUpdated Vector of Unified RS = ");

					for ( const auto& k : rsUnif ) {
					//cout << get<0>(k) << get<1>(k) << get<2>(k) << get<3>(k) << get<4>(k) << get<5>(k) << get<6>(k) << get<7>(k) << get<8>(k) << get<9>(k) << get<10>(k) <<get<11>(k) <<get<12>(k) <<get<13>(k) << endl;
					}
					
					//printf("\n\n");
					
				}
				
				
				
				
			}
			
		break;}
		
		default: break;
	}
	n_max_completed = max(n_max_completed, n_completed);
	p_stats-> max_completed = std::max(n_max_completed, p_stats-> max_completed);
	
}

/**
 * Subroutine for completing the scheduler and getting any final stats
 * XXX You're responsible for completing this routine.
 *
 * @param[out]  p_stats         Pointer to the stats structure
 */
void scheduler_complete(scheduler_stats_t* p_stats) {
	p_stats-> num_insts = n_insts;
	p_stats-> num_cycles = n_cycles;
	p_stats-> ipc = (double)n_insts/n_cycles;
	p_stats-> issue_stall = n_issue_stall;

}


