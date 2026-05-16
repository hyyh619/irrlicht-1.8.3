import re

body = '''
        VS_OUTPUT output;
        output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
        float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));
        return output;
    '''

body = body.strip()
print(f'Body after strip: [{body}]')

statements = []
current_stmt = []
brace_count = 0

for char in body:
    if char == '{':
        brace_count += 1
        current_stmt.append(char)
    elif char == '}':
        brace_count -= 1
        current_stmt.append(char)
    elif char == ';' and brace_count == 0:
        stmt = ''.join(current_stmt).strip()
        if stmt:
            statements.append(stmt)
        current_stmt = []
    else:
        current_stmt.append(char)

if current_stmt:
    stmt = ''.join(current_stmt).strip()
    if stmt:
        statements.append(stmt)

print(f'Statements: {len(statements)}')
for i, s in enumerate(statements):
    print(f'  {i}: [{s}]')