# 2026.5.20
## 1. before re pattern optimization
interpreter.interpret():             0.1699s
interpreter.load_vs_output_golden_from_csv(): 0.0373s
interpreter.executeVS():           7.9531s
compare_vs_output_with_golden():    0.0177s
Total execution time:               8.2995s

## 2. re pattern optimization
interpreter.interpret():             0.0674s
interpreter.load_vs_output_golden_from_csv(): 0.0339s
interpreter.executeVS():           7.5078s
compare_vs_output_with_golden():    0.0178s
Total execution time:               7.7497s

interpreter.interpret():             0.0691s
interpreter.load_vs_output_golden_from_csv(): 0.0350s
interpreter.executeVS():           7.3535s
compare_vs_output_with_golden():    0.0182s
Total execution time:               7.5974s

## 3. cache parser functions
_find_top_level_operator_cached
_is_proper_paren
_find_ternary_colon

interpreter.interpret():             0.0703s
interpreter.load_vs_output_golden_from_csv(): 0.0373s
interpreter.executeVS():           4.8450s
compare_vs_output_with_golden():    0.0174s
Total execution time:               5.0917s

interpreter.interpret():             0.0727s
interpreter.load_vs_output_golden_from_csv(): 0.0372s
interpreter.executeVS():           4.9218s
compare_vs_output_with_golden():    0.0175s
Total execution time:               5.1734s

## 4. Close syntax tree print
interpreter.interpret():             0.1528s
interpreter.load_vs_output_golden_from_csv(): 0.0346s
interpreter.executeVS():           4.3066s
compare_vs_output_with_golden():    0.0178s
Total execution time:               4.6328s

interpreter.interpret():             0.0703s
interpreter.load_vs_output_golden_from_csv(): 0.0347s
interpreter.executeVS():           4.4963s
compare_vs_output_with_golden():    0.0175s
Total execution time:               4.7444s

## 5. Create vertex object
git commit: hlsl-inter: create vertex object by MiniMax-M2.7.
interpreter.interpret():             0.1886s
interpreter.load_vs_output_golden_from_csv(): 0.0401s
interpreter.executeVS():           5.0643s
compare_vs_output_with_golden():    0.0306s
Total execution time:               5.4550s

interpreter.interpret():             0.0920s
interpreter.load_vs_output_golden_from_csv(): 0.0390s
interpreter.executeVS():           5.0451s
compare_vs_output_with_golden():    0.0298s
Total execution time:               5.3428s