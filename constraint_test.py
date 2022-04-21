#-------------------------------------------------------------------------------------------------
# Valid modbus byte formats are:
# SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
# SERIAL_8E1: 1 start bit, 8 data bits, 1 Even parity bit, 1 stop bit
# SERIAL_8O1: 1 start bit, 8 data bits, 1 Odd parity bit, 1 stop bit
#
# Create a constraint that limits the possible options to 10 bits
#
# pip install python-constraint
#-------------------------------------------------------------------------------------------------
import constraint

def create_constraints():
    
    problem = constraint.Problem()

    problem.addVariable('databits', [7,8])  
    problem.addVariable('startbits', [1])  
    problem.addVariable('stopbits', [1,2])  
    problem.addVariable('parity', [0,1,2])  # 0=none, 1=even, 2=odd

    def our_constraint(x, y, z, w):
        if x + y + z + w <= 10:
            return True

    problem.addConstraint(our_constraint, ['databits','startbits','stopbits','parity'])

    sol = problem.getSolutions()       
    for s in sol:
        yield s
    
def get_next():
    global solution
    mylist=[0,0,0,0]
    try:
        s=next(solution)
        mylist=[s['databits'], s['startbits'], s['stopbits'],  s['parity']]
        return mylist 
    except StopIteration :
        print("Last interation")
  
    return mylist


solution=create_constraints()
print("solution created")
