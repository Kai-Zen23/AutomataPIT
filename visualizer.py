import tkinter as tk
from tkinter import ttk, messagebox, PhotoImage
import subprocess
import os
import sys

class AutomataVisualizer:
    def __init__(self, root):
        self.root = root
        self.root.title("Automata Compiler Visualizer")
        self.root.geometry("1000x700")

        # Config
        self.exe_path = "main.exe"
        if not os.path.exists(self.exe_path):
             # Try checking if it's in src/ (unlikely given build command but safe to check)
             if os.path.exists(os.path.join("src", "main.exe")):
                 self.exe_path = os.path.join("src", "main.exe")

        # Styles
        style = ttk.Style()
        style.theme_use('clam')

        # Control Panel
        control_frame = ttk.LabelFrame(root, text="Input", padding="10")
        control_frame.pack(fill=tk.X, padx=10, pady=5)

        # Input Row
        input_frame = ttk.Frame(control_frame)
        input_frame.pack(fill=tk.X, pady=2)

        # Mode Selection
        mode_frame = ttk.Frame(control_frame)
        mode_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(mode_frame, text="Mode: ").pack(side=tk.LEFT)
        self.mode_var = tk.IntVar(value=1)
        ttk.Radiobutton(mode_frame, text="Regex Analysis (NFA/DFA)", variable=self.mode_var, value=1).pack(side=tk.LEFT, padx=5)
        ttk.Radiobutton(mode_frame, text="Calculator (PDA)", variable=self.mode_var, value=3).pack(side=tk.LEFT, padx=5)

        ttk.Label(input_frame, text="Enter Input:").pack(side=tk.LEFT)
        self.regex_entry = ttk.Entry(input_frame, width=40, font=('Consolas', 11))
        self.regex_entry.pack(side=tk.LEFT, padx=10)
        self.regex_entry.insert(0, "(a|b)*abb")
        self.regex_entry.bind('<Return>', lambda e: self.run_visualization())

        self.btn_run = ttk.Button(input_frame, text="Generate Automata", command=self.run_visualization)
        self.btn_run.pack(side=tk.LEFT, padx=10)

        # Symbol Toolbar
        toolbar_frame = ttk.Frame(control_frame)
        toolbar_frame.pack(fill=tk.X, pady=2)
        
        symbols = ['*', '|', '(', ')', '+', '?', '[a-z]', '[0-9]']
        for sym in symbols:
            btn = ttk.Button(toolbar_frame, text=sym, width=5, command=lambda s=sym: self.insert_symbol(s))
            btn.pack(side=tk.LEFT, padx=2)


        # Status Bar
        self.status_var = tk.StringVar()
        self.status_var.set("Ready")
        self.status_bar = ttk.Label(root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)

        # Tabs for Images
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.tab_nfa = self.create_tab("NFA")
        self.tab_dfa = self.create_tab("DFA")
        self.tab_min = self.create_tab("Minimized DFA")
        self.tab_console = self.create_text_tab("Console Output")

        # Dictionary to hold PhotoImage references to prevent garbage collection
        self.images = {}

    def create_text_tab(self, title):
        frame = ttk.Frame(self.notebook)
        self.notebook.add(frame, text=title)
        
        text_area = tk.Text(frame, wrap="word", font=('Consolas', 10))
        scroll_y = ttk.Scrollbar(frame, orient="vertical", command=text_area.yview)
        text_area.configure(yscrollcommand=scroll_y.set)
        
        scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        text_area.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        frame.text_area = text_area
        return frame

    def create_tab(self, title):
        frame = ttk.Frame(self.notebook)
        self.notebook.add(frame, text=title)
        
        # Scrollable image area
        canvas = tk.Canvas(frame, bg='#f0f0f0')
        scroll_x = ttk.Scrollbar(frame, orient="horizontal", command=canvas.xview)
        scroll_y = ttk.Scrollbar(frame, orient="vertical", command=canvas.yview)
        canvas.configure(xscrollcommand=scroll_x.set, yscrollcommand=scroll_y.set)

        scroll_x.pack(side=tk.BOTTOM, fill=tk.X)
        scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Store canvas reference in the frame for easy access later
        frame.canvas = canvas
        return frame

    def update_image(self, tab, filename):
        if not os.path.exists(filename):
            return

        try:
            img = PhotoImage(file=filename)
            self.images[filename] = img # Keep reference
            
            canvas = tab.canvas
            canvas.delete("all")
            canvas.create_image(0, 0, image=img, anchor="nw")
            canvas.config(scrollregion=canvas.bbox("all"))
        except Exception as e:
            messagebox.showerror("Image Load Error", f"Failed to load {filename}: {e}")

    def insert_symbol(self, symbol):
        self.regex_entry.insert(tk.INSERT, symbol)
        self.regex_entry.focus()

    def run_visualization(self):
        regex = self.regex_entry.get()
        if not regex:
            return

        self.status_var.set("Running Compiler...")
        self.root.update()

        # 1. Run main.exe
        # 1. Run main.exe
        try:
            mode = self.mode_var.get()
            # Input: Mode -> Input -> Enter
            input_str = f"{mode}\n{regex}\n"
            
            # Run hidden
            startupinfo = None
            if os.name == 'nt':
                startupinfo = subprocess.STARTUPINFO()
                startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW

            proc = subprocess.run([self.exe_path], input=input_str, text=True, capture_output=True, check=True, startupinfo=startupinfo)
            
            # Display Output
            self.tab_console.text_area.delete(1.0, tk.END)
            self.tab_console.text_area.insert(tk.END, proc.stdout)
            
            if mode == 3:
                self.notebook.select(self.tab_console)
                self.status_var.set("PDA Simulation Complete.")
                return # Skip graph generation for calculator
                
        except Exception as e:
            self.status_var.set("Error running compiler")
            messagebox.showerror("Execution Error", f"Failed to run {self.exe_path}\n{e}")
            return

        # 2. Convert DOT to PNG
        self.status_var.set("Generating Graphs...")
        self.root.update()
        
        files = [
            ("nfa.dot", "nfa.png"),
            ("dfa.dot", "dfa.png"),
            ("min_dfa.dot", "min_dfa.png")
        ]

        try:
            for dot_file, png_file in files:
                if os.path.exists(dot_file):
                    # Use 'dot' from Graphviz
                    cmd = ["dot", "-Tpng", dot_file, "-o", png_file]
                    subprocess.run(cmd, check=True, startupinfo=startupinfo)
        except FileNotFoundError:
             self.status_var.set("Graphviz not found")
             messagebox.showerror("Graphviz Error", "Graphviz 'dot' command not found.\nPlease install Graphviz and add it to PATH.")
             return
        except subprocess.CalledProcessError as e:
             self.status_var.set("Graphviz error")
             messagebox.showerror("Graphviz Error", f"Failed to generate image: {e}")
             return

        # 3. Update UI
        self.update_image(self.tab_nfa, "nfa.png")
        self.update_image(self.tab_dfa, "dfa.png")
        self.update_image(self.tab_min, "min_dfa.png")
        
        self.status_var.set("Done.")

if __name__ == "__main__":
    if not os.path.exists("main.exe") and not os.path.exists("src/main.exe"):
        messagebox.showwarning("Missing Executable", "Could not find main.exe. Please compile the C++ project first.")
    
    root = tk.Tk()
    app = AutomataVisualizer(root)
    root.mainloop()
