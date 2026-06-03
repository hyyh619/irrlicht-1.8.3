import sys
sys.path.insert(0, '.')
from hlsl_interpreter import HLSLInterpreter

interpreter = HLSLInterpreter()
interpreter.variables['WorldViewProj'] = [
    [1.03104, 0.00000, -0.05065, 24.85304],
    [0.00476, 1.37295, 0.09699, -98.08849],
    [0.04896, -0.07058, 0.99664, 125.71310],
    [0.04895, -0.07055, 0.99631, 126.67120]
]

local_vars = {}

# Test transpose directly
expr = 'transpose(WorldViewProj)'
print('Testing:', expr)
import re
match = re.search(r'transpose\s*\(([^)]+)\)', expr)
if match:
    print('matched arg:', match.group(1))
    val = interpreter.get_value(match.group(1), local_vars)
    print('get_value result:', val)

# Test evaluate_expression
result = interpreter.evaluate_expression(expr, local_vars)
print('evaluate_expression result:', result)