import re

local_vars = {
    'input.Pos': [0.45654, 8.08734, 2.19389],
    'input.Normal': [-0.05957, -0.53071, 0.84485],
    'input.Color': [0.8, 0.8, 0.8, 1.0],
    'input.TexCoord': [0.0, 0.0],
    'output': {'Pos': None, 'Color': None, 'TexCoord': None, 'TexCoord2': None, 'Normal': None, 'WorldPos': None}
}

WorldViewProj = [
    [1.03104, 0.00000, -0.05065, 24.85304],
    [0.00476, 1.37295, 0.09699, -98.08849],
    [0.04896, -0.07058, 0.99664, 125.71310],
    [0.04895, -0.07055, 0.99631, 126.67120]
]

# float4(input.Pos, 1.0)
input_pos = local_vars['input.Pos']
float4_input = input_pos + [1.0]
print(f'float4_input: {float4_input}')

# mul(float4(input.Pos, 1.0), transpose(WorldViewProj))
def transpose_matrix(m):
    return [[m[j][i] for j in range(4)] for i in range(4)]

def mul_matrix_vector(m, v):
    if not v:
        return [0, 0, 0, 0]
    result = []
    for row in m:
        s = sum(row[i] * v[i] for i in range(len(v)))
        result.append(s)
    return result

transposed = transpose_matrix(WorldViewProj)
print(f'transposed WorldViewProj: {transposed}')

mul_result = mul_matrix_vector(transposed, float4_input)
print(f'mul result: {mul_result}')

# assign to output.Pos
local_vars['output']['Pos'] = mul_result
print(f'output.Pos: {local_vars["output"]["Pos"]}')