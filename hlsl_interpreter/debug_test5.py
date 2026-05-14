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

# Test evaluating expression without 'transpose' keyword
expr1 = 'WorldViewProj'
result1 = interpreter.evaluate_expression(expr1, local_vars)
print('evaluate_expression(WorldViewProj):', result1)

# Test evaluating expression with transpose
expr2 = 'transpose(WorldViewProj)'
result2 = interpreter.evaluate_expression(expr2, local_vars)
print('evaluate_expression(transpose(WorldViewProj)):', result2)

# Direct test
m = interpreter.get_value('WorldViewProj', local_vars)
print('WorldViewProj matrix:', m)
transposed = interpreter.transpose_matrix(m)
print('transposed:', transposed)