---------------------------------------------------------------------------------------------------
-- Simple FLI demo calling a Python function from VHDL
-- A constraint solver is called which gives all valid solutions for a modbus serial frame
--
-- https://github.com/htminuslab            
--                                       
---------------------------------------------------------------------------------------------------
--                                                    
---------------------------------------------------------------------------------------------------
-- 
--  Revision History:                                                        
--                                                                           
--  Date:          Revision    Author         
--  14 June  2017  0.1         Hans Tiggeler 
--  20 April 2022  0.2         Hans Tiggeler  Updated for Python3.8/Modelsim 2022.1 
---------------------------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package pkg is

	type intarray is array(1 to 4) of integer;

	procedure call_python_module(module_name:in string);
	attribute FOREIGN of call_python_module : procedure is "call_python_module ./fli_python.dll";	
		
	procedure call_python_function(function_name:in string; vhdl_array:OUT intarray);
	attribute FOREIGN of call_python_function : procedure is "call_python_function ./fli_python.dll";			
end;

package body pkg is	
	procedure call_python_module(module_name:in string) is begin end;
	procedure call_python_function(function_name:in string; vhdl_array:OUT intarray) is begin end;
end;


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb is
end tb;

use work.pkg.all;

architecture example of tb is

begin

  initproc: process
	variable retval : intarray:=(others=>0);
  begin
    call_python_module("constraint_test");	
	wait for 100 ns;
	
	for i in 1 to 4 loop
        call_python_function("get_next",retval);
		
		report "Constraint databits=" & to_string(retval(1)) &
			   " startbits=" & to_string(retval(2)) &
			   " stopbits="  & to_string(retval(3)) &
			   " parity="    & to_string(retval(4));
		
		wait for 100 ns;
    end loop;
	
	wait;
  end process;

end;

