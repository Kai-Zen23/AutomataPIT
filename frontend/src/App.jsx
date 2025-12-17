import { useState, useEffect, useRef } from 'react';

const API_URL = import.meta.env.VITE_API_URL || 'http://localhost:8000';

import GraphVisualizer from './components/GraphVisualizer';

function App() {
  const [mode, setMode] = useState(3); // 1: Regex, 3: Calculator
  const [input, setInput] = useState('x = 5 + 3');
  const [testString, setTestString] = useState('aabb'); // Default test string
  const [output, setOutput] = useState('');
  const [trace, setTrace] = useState([]);
  const [graphs, setGraphs] = useState({}); // { nfa: "...", dfa: "...", min_dfa: "..." }
  const [viewMode, setViewMode] = useState('trace'); // 'trace' | 'nfa' | 'dfa' | 'min_dfa'
  const [step, setStep] = useState(-1);
  const [isPlaying, setIsPlaying] = useState(false);
  const [speed, setSpeed] = useState(500);
  const [loading, setLoading] = useState(false);
  const [history, setHistory] = useState([]);
  const playRef = useRef(null);

  const fetchHistory = async () => {
    try {
      const res = await fetch(`${API_URL}/history`);
      if (res.ok) {
        const data = await res.json();
        setHistory(data);
      }
    } catch (e) {
      console.error("Failed to fetch history", e);
    }
  };

  useEffect(() => {
    fetchHistory();
  }, []);

  // Auto-play loop
  useEffect(() => {
    if (isPlaying) {
      playRef.current = setInterval(() => {
        setStep((prev) => {
          if (prev < trace.length - 1) return prev + 1;
          setIsPlaying(false);
          return prev;
        });
      }, speed);
    } else {
      clearInterval(playRef.current);
    }
    return () => clearInterval(playRef.current);
  }, [isPlaying, trace.length, speed]);

  // Parse active state from current trace step
  const getActiveStateId = () => {
    if (step < 0 || !trace[step]) return null;
    const currentAction = trace[step];
    // Parser/Backend returns trace objects.
    // For Regex (DFA), we need to extract "qX" from the action or stack info.
    // Our backend parser.py might need to be checked, but usually returns 'stack_content' like "[q0]"

    const stackContent = currentAction.data?.stack_content || "";
    // content is like "[q0]"
    const match = stackContent.match(/q(\d+)/);
    if (match) {
      return match[1];
    }
    return null;
  };

  const handleSimulate = async () => {
    setLoading(true);
    setTrace([]);
    setGraphs({});
    setStep(-1);
    setIsPlaying(false);
    setOutput('');

    try {
      const res = await fetch(`${API_URL}/simulate`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mode, input_text: input, test_string: testString }),
      });
      const data = await res.json();

      if (data.error) {
        setOutput(`Error: ${data.error}\n${data.stderr || ''}`);
      } else {
        setOutput(data.stdout + (data.stderr ? `\nSTDERR:\n${data.stderr}` : ''));
        if (data.trace) {
          setTrace(data.trace);
          if (data.trace.length > 0) setStep(-1);
        }
        if (data.graphs) {
          setGraphs(data.graphs);
          // Auto switch to DFA view if available
          if (data.graphs.dfa) setViewMode('dfa');
          fetchHistory(); // Refresh history list
        }
      }
    } catch (err) {
      setOutput(`Network Error: ${err.message}`);
    } finally {
      setLoading(false);
    }
  };

  const getVisibleStack = () => {
    if (step < 0 || !trace[step] || trace[step].type !== 'stack') return [];
    // In a real implementation we would accumulate the stack actions
    // But our parser currently returns "Stack Content" as a string in the trace event
    // So we can just show the current row.
    return trace.slice(0, step + 1).filter(t => t.type === 'stack');
  };

  const getVisibleTokens = () => {
    // Show tokens accumulated up to current step
    if (step < 0) return [];
    // This logic depends on when tokens appear in the trace vs stack actions
    // For now, let's just show all tokens that have appeared so far
    return trace.slice(0, step + 1).filter(t => t.type === 'token');
  }

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 p-8 font-sans selection:bg-sky-500/30">
      <div className="max-w-7xl mx-auto space-y-8">

        {/* Header */}
        <header className="flex items-center justify-between border-b border-slate-800 pb-6">
          <div>
            <h1 className="text-3xl font-bold bg-gradient-to-r from-sky-400 to-indigo-400 bg-clip-text text-transparent">
              Automata Visualizer
            </h1>
            <p className="text-slate-400 mt-2">Interactive C++ Compiler Frontend Simulator</p>
          </div>
          <div className="flex space-x-4 bg-slate-900 p-1 rounded-lg border border-slate-800">
            <button
              onClick={() => setMode(1)}
              className={`px-4 py-2 rounded-md transition-all ${mode === 1 ? 'bg-sky-600 text-white shadow-lg shadow-sky-900/20' : 'text-slate-400 hover:text-white'}`}
            >
              Regex Analysis
            </button>
            <button
              onClick={() => setMode(3)}
              className={`px-4 py-2 rounded-md transition-all ${mode === 3 ? 'bg-sky-600 text-white shadow-lg shadow-sky-900/20' : 'text-slate-400 hover:text-white'}`}
            >
              Calculator (PDA)
            </button>
          </div>
        </header>

        {/* Input Area */}
        <section className="bg-slate-900/50 rounded-xl border border-slate-800 p-6 backdrop-blur-sm space-y-4">

          <div className="flex gap-4 items-start">
            <div className="flex-1 space-y-1">
              <label className="block text-sm font-medium text-slate-400">
                {mode === 1 ? 'Regex Pattern' : 'Expression'}
              </label>
              <input
                type="text"
                value={input}
                onChange={(e) => setInput(e.target.value)}
                className="w-full bg-slate-950 border border-slate-700 rounded-lg px-4 py-3 font-mono text-lg focus:outline-none focus:ring-2 focus:ring-sky-500/50 transition-all placeholder-slate-600"
                placeholder={mode === 1 ? "(a|b)abb" : "x = 5 + 3"}
              />
            </div>

            {mode === 1 && (
              <div className="flex-1 space-y-1">
                <label className="block text-sm font-medium text-slate-400">Test String</label>
                <input
                  type="text"
                  value={testString}
                  onChange={(e) => setTestString(e.target.value)}
                  className="w-full bg-slate-950 border border-slate-700 rounded-lg px-4 py-3 font-mono text-lg focus:outline-none focus:ring-2 focus:ring-sky-500/50 transition-all placeholder-slate-600"
                  placeholder="aabb"
                  onKeyDown={(e) => e.key === 'Enter' && handleSimulate()}
                />
              </div>
            )}

            <div className="flex items-end self-end">
              <button
                onClick={handleSimulate}
                disabled={loading}
                className="bg-sky-600 hover:bg-sky-500 text-white px-8 py-3 rounded-lg font-semibold transition-all disabled:opacity-50 disabled:cursor-not-allowed shadow-lg shadow-sky-900/20 flex items-center gap-2 h-[54px]"
              >
                {loading ? 'Compiling...' : 'Visualize'}
              </button>
            </div>
          </div>

          {/* Helper Examples */}
          <div className="flex gap-2 items-center text-sm">
            <span className="text-slate-500 font-medium">Examples:</span>
            {mode === 1 ? (
              <>
                <button onClick={() => { setInput('[a-zA-Z_][a-zA-Z0-9]*'); setTestString('myVar_1'); }} className="px-2 py-1 bg-slate-800 hover:bg-slate-700 text-sky-400 rounded border border-slate-700 transition-colors">
                  Valid Identifier
                </button>
                <button onClick={() => { setInput('(a|b)*abb'); setTestString('ababb'); }} className="px-2 py-1 bg-slate-800 hover:bg-slate-700 text-sky-400 rounded border border-slate-700 transition-colors">
                  Ends with abb
                </button>
                <button onClick={() => { setInput('[0-9]+(\\.[0-9]+)?'); setTestString('3.1415'); }} className="px-2 py-1 bg-slate-800 hover:bg-slate-700 text-sky-400 rounded border border-slate-700 transition-colors">
                  Floating Point
                </button>
              </>
            ) : (
              <>
                <button onClick={() => setInput('x = 5 + 3')} className="px-2 py-1 bg-slate-800 hover:bg-slate-700 text-indigo-400 rounded border border-slate-700 transition-colors">
                  Simple Assign
                </button>
                <button onClick={() => setInput('res = ( 5 + 3 ) * 2')} className="px-2 py-1 bg-slate-800 hover:bg-slate-700 text-indigo-400 rounded border border-slate-700 transition-colors">
                  Parentheses
                </button>
                <button onClick={() => setInput('val = 10 + 5 * 2')} className="px-2 py-1 bg-slate-800 hover:bg-slate-700 text-indigo-400 rounded border border-slate-700 transition-colors">
                  Precedence
                </button>
              </>
            )}
          </div>
        </section>

        {/* Main Workspace */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 h-[700px]">

          {/* Left: Visualization (Graph or Trace) */}
          <section className="bg-slate-900 rounded-xl border border-slate-800 flex flex-col overflow-hidden shadow-2xl">
            <div className="p-2 border-b border-slate-800 flex justify-between items-center bg-slate-900/80">
              {/* View Tabs */}
              <div className="flex space-x-1 bg-slate-950/50 p-1 rounded-lg">
                <button
                  onClick={() => setViewMode('trace')}
                  className={`px-3 py-1.5 rounded text-xs font-semibold overflow-hidden transition-all ${viewMode === 'trace' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}
                >
                  Trace Table
                </button>
                {graphs.nfa && (
                  <button
                    onClick={() => setViewMode('nfa')}
                    className={`px-3 py-1.5 rounded text-xs font-semibold overflow-hidden transition-all ${viewMode === 'nfa' ? 'bg-sky-800/50 text-sky-200 shadow-sm border border-sky-700/50' : 'text-slate-500 hover:text-sky-400'}`}
                  >
                    NFA Graph
                  </button>
                )}
                {graphs.dfa && (
                  <button
                    onClick={() => setViewMode('dfa')}
                    className={`px-3 py-1.5 rounded text-xs font-semibold overflow-hidden transition-all ${viewMode === 'dfa' ? 'bg-emerald-800/50 text-emerald-200 shadow-sm border border-emerald-700/50' : 'text-slate-500 hover:text-emerald-400'}`}
                  >
                    DFA Graph
                  </button>
                )}
                {graphs.min_dfa && (
                  <button
                    onClick={() => setViewMode('min_dfa')}
                    className={`px-3 py-1.5 rounded text-xs font-semibold overflow-hidden transition-all ${viewMode === 'min_dfa' ? 'bg-indigo-800/50 text-indigo-200 shadow-sm border border-indigo-700/50' : 'text-slate-500 hover:text-indigo-400'}`}
                  >
                    Min DFA
                  </button>
                )}
                {graphs.pda && (
                  <button
                    onClick={() => setViewMode('pda')}
                    className={`px-3 py-1.5 rounded text-xs font-semibold overflow-hidden transition-all ${viewMode === 'pda' ? 'bg-purple-800/50 text-purple-200 shadow-sm border border-purple-700/50' : 'text-slate-500 hover:text-purple-400'}`}
                  >
                    PDA Graph
                  </button>
                )}
              </div>

              <div className="flex items-center gap-2">
                <div className="px-3 py-1 bg-slate-800 rounded text-xs font-mono text-sky-400 border border-slate-700">
                  Step: {step + 1} / {trace.length}
                </div>
              </div>
            </div>

            <div className="flex-1 overflow-hidden relative bg-[#0B0F19]">
              {viewMode === 'trace' ? (
                <div className="h-full overflow-auto p-4 space-y-2">
                  {trace.length > 0 ? (
                    <>
                      {getVisibleStack().map((t, i) => (
                        <div key={i} className="flex items-center gap-3 p-3 rounded border border-slate-800 bg-slate-900/50 hover:bg-slate-800/50 transition-colors group animate-in fade-in slide-in-from-left-4 duration-300">
                          <div className="w-8 h-8 flex items-center justify-center rounded-full bg-slate-800 text-xs font-mono text-slate-400 group-hover:bg-sky-900/30 group-hover:text-sky-400">
                            {t.data.step}
                          </div>
                          <div className="flex-1 font-mono text-sm">
                            <div className="flex justify-between text-slate-400 text-xs mb-1">
                              <span>Idx: {t.data.input_index}</span>
                              <span className="text-emerald-400">{t.data.action}</span>
                            </div>
                            <div className="text-slate-200 bg-slate-950 px-2 py-1 rounded border border-slate-800/50">
                              {t.data.stack_content}
                            </div>
                          </div>
                        </div>
                      ))}
                      <div ref={(el) => el?.scrollIntoView({ behavior: 'smooth' })} />
                    </>
                  ) : (
                    <div className="h-full flex flex-col items-center justify-center text-slate-600 space-y-4">
                      <div className="w-16 h-16 rounded-full bg-slate-900 border border-slate-800 flex items-center justify-center text-2xl">
                        ⚡
                      </div>
                      <p>Run a simulation to see the trace.</p>
                    </div>
                  )}
                </div>
              ) : (
                <GraphVisualizer
                  dotString={graphs[viewMode]}
                  activeStateId={getActiveStateId()}
                />
              )}
            </div>

            {/* Controls */}
            <div className="p-4 border-t border-slate-800 bg-slate-900/80 backdrop-blur">
              <div className="flex items-center justify-between gap-4">
                <div className="flex gap-2">
                  <button
                    onClick={() => setStep(s => Math.max(-1, s - 1))}
                    disabled={step < 0}
                    className="p-2 rounded hover:bg-slate-800 text-slate-400 hover:text-white disabled:opacity-30 disabled:hover:bg-transparent"
                  >
                    Prev
                  </button>
                  <button
                    onClick={() => setIsPlaying(!isPlaying)}
                    disabled={trace.length === 0}
                    className={`px-6 py-2 rounded font-medium transition-all ${isPlaying ? 'bg-amber-600/20 text-amber-400 border border-amber-600/50' : 'bg-emerald-600 text-white hover:bg-emerald-500 shadow-lg shadow-emerald-900/20'}`}
                  >
                    {isPlaying ? 'Pause' : 'Play'}
                  </button>
                  <button
                    onClick={() => setStep(s => Math.min(trace.length - 1, s + 1))}
                    disabled={step >= trace.length - 1}
                    className="p-2 rounded hover:bg-slate-800 text-slate-400 hover:text-white disabled:opacity-30 disabled:hover:bg-transparent"
                  >
                    Next
                  </button>
                </div>

                <div className="flex items-center gap-3 flex-1 justify-end">
                  <span className="text-xs text-slate-500 uppercase tracking-wider font-semibold">Speed</span>
                  <input
                    type="range"
                    min="100"
                    max="2000"
                    step="100"
                    value={speed}
                    onChange={(e) => setSpeed(Number(e.target.value))}
                    className="w-32 accent-sky-500"
                  />
                </div>
              </div>
            </div>
          </section>

          {/* Center: Console Output */}
          <section className="bg-slate-900 rounded-xl border border-slate-800 flex flex-col overflow-hidden">
            <div className="p-4 border-b border-slate-800 bg-slate-900/80">
              <h2 className="font-semibold text-slate-200">Execution Log</h2>
            </div>
            <div className="flex-1 overflow-auto p-0 bg-slate-950">
              <pre className="p-4 font-mono text-sm text-slate-300 whitespace-pre-wrap leading-relaxed">
                {output || <span className="text-slate-600 italic">// Console output will appear here...</span>}
              </pre>
            </div>
          </section>

        </div>

        {/* Bottom: Recent History */}
        <section className="bg-slate-900 rounded-xl border border-slate-800 p-6">
          <div className="flex justify-between items-center mb-4">
            <h2 className="text-xl font-bold bg-gradient-to-r from-purple-400 to-pink-400 bg-clip-text text-transparent">
              Recent History
            </h2>
            <button onClick={fetchHistory} className="text-sm text-slate-400 hover:text-white underline">
              Refresh
            </button>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
            {history.map((item) => (
              <div
                key={item.id}
                onClick={() => {
                  setMode(item.mode);
                  setInput(item.input_text);
                  setTestString(item.test_string || '');
                  if (item.result_output) setOutput(item.result_output);
                  window.scrollTo({ top: 0, behavior: 'smooth' });
                }}
                className="bg-slate-950 border border-slate-800 p-4 rounded-lg cursor-pointer hover:bg-slate-800 transition-all group"
              >
                <div className="flex justify-between items-start mb-2">
                  <span className={`text-xs px-2 py-0.5 rounded font-mono ${item.mode === 1 ? 'bg-sky-900/50 text-sky-300' : 'bg-indigo-900/50 text-indigo-300'}`}>
                    {item.mode === 1 ? 'REGEX' : 'CALC'}
                  </span>
                  <span className="text-xs text-slate-500">
                    {new Date(item.created_at).toLocaleTimeString()}
                  </span>
                </div>
                <p className="font-mono text-sm text-slate-300 truncate mb-1" title={item.input_text}>
                  {item.input_text}
                </p>
                {item.test_string && (
                  <p className="font-mono text-xs text-slate-500 truncate">
                    Test: {item.test_string}
                  </p>
                )}
              </div>
            ))}
            {history.length === 0 && (
              <div className="col-span-full text-center py-8 text-slate-500 italic">
                No history yet. Run a simulation to see it here.
              </div>
            )}
          </div>
        </section>
      </div>
    </div>
  );
}

export default App;
