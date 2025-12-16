import re

def parse_trace(output: str):
    trace = []
    token_counter = 0
    
    # Temporary buffers for multi-line step parsing
    current_step = None
    current_idx = None
    current_action = ""
    
    lines = output.splitlines()
    for i, line in enumerate(lines):
        line = line.strip()
        
        # 1. Parse Tokens: "Token: TYPE (VALUE)"
        if line.startswith("Token:"): 
            m = re.search(r"Token: (\w+) \((.*)\)", line)
            if m:
                trace.append({
                    'type': 'token',
                    'data': {
                        'id': token_counter + 1,
                        'token_type': m.group(1),
                        'value': m.group(2)
                    }
                })
                token_counter += 1
        
        # 2. Parse Stack Step Header: "Step 1: Read 'x' at position 0"
        elif line.startswith("Step"): 
            m = re.search(r"Step (\d+): Read .* at position (\d+)", line)
            if m:
                current_step = int(m.group(1))
                current_idx = int(m.group(2))
                current_action = "" # Reset action

        # 3. Parse Action: "Action: PUSH ..."
        elif line.startswith("Action:"):
             current_action = line.replace("Action:", "").strip()

        # 4. Parse Stack: "Stack: [...]" -> Commit the row
        elif line.startswith("Stack: ["):
            if current_step is not None and current_idx is not None:
                stack_content = line.replace("Stack: [", "").replace("]", "").strip()
                trace.append({
                    'type': 'stack',
                    'data': {
                        'step': current_step,
                        'input_index': current_idx,
                        'action': current_action,
                        'stack_content': stack_content
                    }
                })
                # Prepare for next
                current_step = None 
                
    return trace
