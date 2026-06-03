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

local_vars = {
    'input.Pos': [0.45654, 8.08734, 2.19389],
    'input.Normal': [-0.05957, -0.53071, 0.84485],
}

expr = 'mul(float4(input.Pos, 1.0), transpose(WorldViewProj))'
print('Expression:', expr)
print('Has mul:', 'mul' in expr)
print('Has float4:', 'float4' in expr)

# Test float4 evaluation
expr2 = 'float4(input.Pos, 1.0)'
result2 = interpreter.evaluate_expression(expr2, local_vars)
print('float4 result:', result2)

# Test transpose evaluation
expr3 = 'transpose(WorldViewProj)'
result3 = interpreter.evaluate_expression(expr3, local_vars)
print('transpose result:', result3)