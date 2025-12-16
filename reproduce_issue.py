import subprocess
import re
import os

exe_path = "compiler_frontend.exe"
input_str = "3\nx = 5 + 3\n"

print("Running compiler...")
try:
    proc = subprocess.run([exe_path], input=input_str, text=True, capture_output=True, check=True)
    output_lines = proc.stdout.splitlines()
    
    print("Output captured. Parsing...")
    
    token_count = 0
    stack_count = 0
    
    for line in output_lines:
        line = line.strip()
        # Parse Token
        token_match = re.search(r"Token: (\w+) \((.*)\)", line)
        if token_match:
            print(f"MATCH TOKEN: {token_match.groups()}")
            token_count += 1
            
        # Parse Stack
        stack_match = re.search(r"State: (\d+), InputIdx: (\d+), Stack: (.*)", line)
        if stack_match:
            print(f"MATCH STACK: {stack_match.groups()}")
            stack_count += 1
            
    print(f"Total Tokens: {token_count}")
    print(f"Total Stack States: {stack_count}")

except subprocess.CalledProcessError as e:
    print(f"Error: {e}")
