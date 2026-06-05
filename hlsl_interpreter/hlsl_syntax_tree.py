import re
from functools import lru_cache
from typing import Any, Dict, List, Optional, Tuple


_COMPILED_PATTERNS: Dict[str, re.Pattern] = {
    'type_cast': re.compile(r'\((\w+)\)\s*(.+)', re.DOTALL),
    'float_constructor': re.compile(r'float[234]\s*\('),
    'function_call': re.compile(r'\w+\s*\('),
    'function_call_format': re.compile(r'^(\w+)\s*\('),
    'method_call': re.compile(r'^(\w+(?:\.\w+)*)\s*\('),
}

_OPERATORS: Dict[str, int] = {
    '||': 1, '&&': 2,
    '|': 3, '&': 4,
    '==': 5, '!=': 5,
    '<': 6, '>': 6, '<=': 6, '>=': 6,
    '+': 7, '-': 7,
    '*': 8, '/': 8,
}


@lru_cache(maxsize=256)
def _split_args_cached(args_str: str) -> Tuple[str, ...]:
    if not args_str.strip():
        return ()
    args = []
    depth = 0
    current = ''
    for char in args_str:
        if char == '(':
            depth += 1
            current += char
        elif char == ')':
            depth -= 1
            current += char
        elif char == ',' and depth == 0:
            args.append(current.strip())
            current = ''
        else:
            current += char
    if current.strip():
        args.append(current.strip())
    return tuple(args)


@lru_cache(maxsize=256)
def _find_top_level_operator_cached(expr: str) -> Optional[Tuple[int, str]]:
    depth = 0
    candidates = []
    i = 0
    while i < len(expr):
        char = expr[i]

        if char == '(':
            depth += 1

        elif char == ')':
            depth -= 1

        elif depth == 0:
            if i >= 1:
                two_char = expr[i-1:i+1]
                if two_char in _OPERATORS:
                    candidates.append((i-1, two_char, _OPERATORS[two_char]))
                    i += 1
                    continue

            two_char = expr[i:i+2]
            if char in _OPERATORS and not (i >= 1 and two_char in _OPERATORS):
                # Skip unary +/- that follows an operator or opening delimiter
                if char in ('+', '-') and i >= 1:
                    j = i - 1
                    while j >= 0 and expr[j] == ' ':
                        j -= 1
                    if j >= 0 and expr[j] in '+-*/%(,[|&!<>=':
                        i += 1
                        continue
                candidates.append((i, char, _OPERATORS[char]))

        i += 1

    if not candidates:
        return None

    min_prec = min(c[2] for c in candidates)
    rightmost = max(c[0] for c in candidates if c[2] == min_prec)
    for c in candidates:
        if c[0] == rightmost and c[2] == min_prec:
            return (c[0], c[1])
    return None


@lru_cache(maxsize=256)
def _is_proper_paren(inner: str) -> bool:
    paren_depth = 0
    for c in inner:
        if c == '(':
            paren_depth += 1
        elif c == ')':
            paren_depth -= 1
        if paren_depth < 0:
            return False
    return True


@lru_cache(maxsize=256)
def _find_ternary_colon(expr: str, start: int) -> int:
    depth = 0
    for i in range(start, len(expr)):
        char = expr[i]
        if char == '(':
            depth += 1
        elif char == ')':
            depth -= 1
        elif char == ':' and depth == 0:
            return i
    return -1


class SyntaxTreeNode:
    """
    HLSL语法树节点基类
    用于表示HLSL表达式解析后的语法树结构
    node_type: 节点类型 - 'value'(值), 'function'(函数), 'binary_op'(二元操作),
                            'unary_op'(一元操作), 'cast'(类型转换), 'ternary'(三元条件)
    value: 节点值 - 变量名/函数名/操作符/类型名
    left: 左子节点 (用于二元/一元操作或三元条件)
    right: 右子节点 (用于二元操作或三元真的表达式)
    third_child: 第三子节点 (用于三元条件假的表达式)
    args: 函数参数列表 (用于函数调用)
    """
    def __init__(self, node_type: str, value: Any = None, left: Optional['SyntaxTreeNode'] = None, right: Optional['SyntaxTreeNode'] = None, third_child: Optional['SyntaxTreeNode'] = None, args: Optional[List['SyntaxTreeNode']] = None, line_number: int = 0):
        self.node_type = node_type
        self.value = value
        self.left = left
        self.right = right
        self.third_child = third_child
        self.args = args if args is not None else []
        self.line_number = line_number

    def __repr__(self):
        return self._pretty(0)

    def _pretty(self, indent: int) -> str:
        prefix = "  " * indent
        if self.node_type == 'function' or self.node_type == 'method_call':
            lines = [f"{self.node_type.capitalize()}({self.value})"]
            if self.left:
                lines.append(f"{prefix}  object:")
                lines.append(self.left._pretty(indent + 2))
            for i, arg in enumerate(self.args):
                lines.append(f"{prefix}  arg[{i}]:")
                lines.append(arg._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'binary_op':
            lines = [f"BinaryOp({self.value})"]
            lines.append(f"{prefix}  left:")
            lines.append(self.left._pretty(indent + 2))
            lines.append(f"{prefix}  right:")
            lines.append(self.right._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'unary_op':
            lines = [f"UnaryOp({self.value})"]
            lines.append(f"{prefix}  child:")
            lines.append(self.left._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'cast':
            lines = [f"Cast({self.value})"]
            lines.append(f"{prefix}  inner:")
            lines.append(self.left._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'ternary':
            lines = [f"Ternary({self.value})"]
            lines.append(f"{prefix}  condition:")
            lines.append(self.left._pretty(indent + 2))
            lines.append(f"{prefix}  true_expr:")
            lines.append(self.right._pretty(indent + 2))
            lines.append(f"{prefix}  false_expr:")
            lines.append(self.third_child._pretty(indent + 2))
            return "\n".join(lines)
        else:
            return f"{prefix}Value({self.value})"


class SyntaxTreeParser:
    """
    HLSL表达式语法树解析器
    负责将HLSL表达式字符串解析为SyntaxTreeNode组成的语法树
    支持: 类型转换、括号表达式、二元运算符、函数调用、变量引用
    """
    def __init__(self):
        pass

    def parse(self, expr: str) -> SyntaxTreeNode:
        expr = expr.strip()
        return self._parse_expression(expr)

    def _find_top_level_operator(self, expr: str) -> Optional[Tuple[int, str]]:
        return _find_top_level_operator_cached(expr)

    def _parse_expression(self, expr: str) -> SyntaxTreeNode:
        expr = expr.strip()
        if not expr:
            return SyntaxTreeNode('value', None)

        # Check for | and & before cast: (int2)a | (int2)b must NOT be treated
        # as a cast of the whole rhs — the bitwise op is at the top level.
        op_info_pre = self._find_top_level_operator(expr)
        if op_info_pre and op_info_pre[1] in ('|', '&'):
            pos, op = op_info_pre
            left_node = self._parse_expression(expr[:pos].strip())
            right_node = self._parse_expression(expr[pos+1:].strip())
            return SyntaxTreeNode('binary_op', op, left_node, right_node)

        cast_match = _COMPILED_PATTERNS['type_cast'].match(expr)
        if cast_match:
            cast_type = cast_match.group(1)
            rest = cast_match.group(2).strip()
            inner_node = self._parse_expression(rest)
            return SyntaxTreeNode('cast', cast_type, inner_node)

        if expr.startswith('(') and expr.endswith(')'):
            inner = expr[1:-1].strip()
            if _is_proper_paren(inner):
                return self._parse_expression(inner)

        ternary_pos = -1
        depth = 0
        for i, char in enumerate(expr):
            if char == '(':
                depth += 1
            elif char == ')':
                depth -= 1
            elif char == '?' and depth == 0:
                ternary_pos = i
                break

        if ternary_pos >= 0:
            colon_pos = _find_ternary_colon(expr, ternary_pos + 1)
            if colon_pos >= 0:
                cond_expr = expr[:ternary_pos].strip()
                true_expr = expr[ternary_pos+1:colon_pos].strip()
                false_expr = expr[colon_pos+1:].strip()
                cond_node = self._parse_expression(cond_expr)
                true_node = self._parse_expression(true_expr)
                false_node = self._parse_expression(false_expr)
                return SyntaxTreeNode('ternary', '?', cond_node, true_node, false_node)

        op_info = self._find_top_level_operator(expr)
        if op_info:
            pos, op = op_info
            if op in ['||', '&&', '|', '&', '==', '!=', '<', '>', '<=', '>=', '+', '-', '*', '/']:
                left_expr = expr[:pos].strip()
                right_expr = expr[pos+len(op):].strip()
                left_node = self._parse_expression(left_expr)
                right_node = self._parse_expression(right_expr)
                return SyntaxTreeNode('binary_op', op, left_node, right_node)

        if _COMPILED_PATTERNS['float_constructor'].match(expr):
            return self._parse_function_call(expr)

        if _COMPILED_PATTERNS['function_call'].match(expr):
            return self._parse_function_call(expr)

        if '.' in expr and '(' in expr:
            return self._parse_method_call(expr)

        return SyntaxTreeNode('value', expr)

    def _parse_function_call(self, expr: str) -> SyntaxTreeNode:
        expr = expr.strip()
        if expr.startswith('('):
            match = _COMPILED_PATTERNS['type_cast'].match(expr)
            if match:
                cast_type = match.group(1)
                rest = match.group(2).strip()
                inner_node = self._parse_expression(rest)
                if inner_node.node_type == 'value':
                    return inner_node
                return SyntaxTreeNode('cast', cast_type, inner_node)

        match = _COMPILED_PATTERNS['function_call_format'].match(expr)
        if not match:
            return SyntaxTreeNode('value', expr)

        func_name = match.group(1)

        depth = 0
        paren_start = -1
        for i, char in enumerate(expr):
            if char == '(':
                depth += 1
                if depth == 1:
                    paren_start = i
            elif char == ')':
                depth -= 1
                if depth == 0:
                    args_str = expr[paren_start+1:i]
                    if func_name in ['transpose', 'normalize', 'length', 'abs', 'sin', 'cos', 'tan']:
                        inner_node = self._parse_expression(args_str.strip())
                        return SyntaxTreeNode('function', func_name, args=[inner_node])
                    elif func_name in ['mul', 'reflect', 'pow', 'max', 'min', 'dot', 'float2', 'float3', 'float4']:
                        arg_nodes = [self._parse_expression(arg.strip()) for arg in _split_args_cached(args_str)]
                        return SyntaxTreeNode('function', func_name, args=arg_nodes)
                    arg_nodes = [self._parse_expression(arg.strip()) for arg in _split_args_cached(args_str)]
                    return SyntaxTreeNode('function', func_name, args=arg_nodes)

        return SyntaxTreeNode('value', expr)

    def _parse_method_call(self, expr: str) -> SyntaxTreeNode:
        """Parse method call expression like DiffuseTexture.Sample(LinearSampler, input.TexCoord)"""
        expr = expr.strip()
        dot_pos = expr.find('.')
        if dot_pos < 0:
            return SyntaxTreeNode('value', expr)

        paren_pos = expr.find('(', dot_pos)
        if paren_pos < 0:
            return SyntaxTreeNode('value', expr)

        obj_name = expr[:dot_pos].strip()
        method_name = expr[dot_pos+1:paren_pos].strip()
        args_str = expr[paren_pos+1:expr.rfind(')')]

        arg_nodes = [self._parse_expression(arg.strip()) for arg in _split_args_cached(args_str)]
        return SyntaxTreeNode('method_call', method_name, left=SyntaxTreeNode('value', obj_name), args=arg_nodes)

    def _split_args(self, args_str: str) -> List[str]:
        return list(_split_args_cached(args_str))