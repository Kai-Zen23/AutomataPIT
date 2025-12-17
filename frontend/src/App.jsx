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
  const [viewMode, setViewMode] = useState('trace'); // 'trace' | 'nfa' | 'dfa' | 'min_dfa' | 'lexer' | 'grammar'
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
    const stackContent = currentAction.data?.stack_content || "";
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
          // Default view modes based on analysis type
          if (mode === 1) setViewMode('nfa'); // Regular Lang -> NFA
          else if (mode === 3) setViewMode('pda'); // Context-Free -> PDA

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
    return trace.slice(0, step + 1).filter(t => t.type === 'stack');
  };

  const getTokens = () => {
    return trace.filter(t => t.type === 'token');
  };

  return (
    <div className="min-h-screen bg-[#050505] text-slate-100 font-sans selection:bg-sky-500/30 selection:text-sky-200">

      {/* Glass Navigation */}
      <nav className="fixed top-0 left-0 right-0 z-50 glass-panel border-b border-white/5 px-6 py-4 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 bg-gradient-to-br from-sky-500 to-indigo-600 rounded-lg flex items-center justify-center shadow-lg shadow-sky-500/20">
            <span className="font-mono font-bold text-white text-lg">A</span>
          </div>
          <div>
            <h1 className="text-lg font-bold tracking-tight text-white">Automata Visualizer</h1>
            <p className="text-[10px] text-slate-400 font-medium uppercase tracking-wider">Formal Language Hierarchy</p>
          </div>
        </div>

        {/* Segmented Control for Mode */}
        <div className="bg-slate-900/50 p-1 rounded-lg border border-white/5 flex gap-1">
          <button
            onClick={() => setMode(1)}
            className={`px-4 py-1.5 rounded-md text-sm font-medium transition-all duration-200 ${mode === 1 ? 'bg-slate-800 text-white shadow-sm ring-1 ring-white/10' : 'text-slate-400 hover:text-slate-200 hover:bg-white/5'}`}
          >
            Lexical Analysis (Regular)
          </button>
          <button
            onClick={() => setMode(3)}
            className={`px-4 py-1.5 rounded-md text-sm font-medium transition-all duration-200 ${mode === 3 ? 'bg-slate-800 text-white shadow-sm ring-1 ring-white/10' : 'text-slate-400 hover:text-slate-200 hover:bg-white/5'}`}
          >
            Syntactic Analysis (Context-Free)
          </button>
        </div>
      </nav>

      <main className="pt-24 pb-12 px-6 max-w-[1600px] mx-auto space-y-6">

        {/* Input Panel */}
        <section className="glass-panel rounded-2xl p-1 relative overflow-hidden group">
          <div className="absolute inset-0 bg-gradient-to-r from-sky-500/10 via-transparent to-transparent opacity-0 group-hover:opacity-100 transition-opacity duration-500 pointer-events-none" />

          <div className="bg-[#0A0A0A]/80 p-6 rounded-xl space-y-6">
            <div className="flex gap-6 items-start">
              <div className="flex-1 space-y-2">
                <label className="text-xs font-semibold text-sky-500 uppercase tracking-wider">
                  {mode === 1 ? 'Regular Expression (Pattern)' : 'Source Code (Input)'}
                </label>
                <div className="relative group/input">
                  <input
                    type="text"
                    value={input}
                    onChange={(e) => setInput(e.target.value)}
                    className="w-full bg-[#050505] border border-white/10 rounded-xl px-5 py-4 font-mono text-lg text-white placeholder-slate-600 focus:outline-none focus:ring-2 focus:ring-sky-500/50 focus:border-sky-500/50 transition-all shadow-inner"
                    placeholder={mode === 1 ? "(a|b)*abb" : "x = 5 + 3"}
                  />
                  <div className="absolute inset-0 rounded-xl ring-1 ring-inset ring-white/5 pointer-events-none group-hover/input:ring-white/10 transition-all" />
                </div>
              </div>

              {mode === 1 && (
                <div className="flex-1 space-y-2">
                  <label className="text-xs font-semibold text-emerald-500 uppercase tracking-wider">Test String</label>
                  <div className="relative group/input">
                    <input
                      type="text"
                      value={testString}
                      onChange={(e) => setTestString(e.target.value)}
                      onKeyDown={(e) => e.key === 'Enter' && handleSimulate()}
                      className="w-full bg-[#050505] border border-white/10 rounded-xl px-5 py-4 font-mono text-lg text-white placeholder-slate-600 focus:outline-none focus:ring-2 focus:ring-emerald-500/50 focus:border-emerald-500/50 transition-all shadow-inner"
                      placeholder="aabb"
                    />
                  </div>
                </div>
              )}

              <div className="pt-8">
                <button
                  onClick={handleSimulate}
                  disabled={loading}
                  className="h-[54px] px-8 bg-sky-600 hover:bg-sky-500 active:bg-sky-700 text-white rounded-xl font-semibold shadow-lg shadow-sky-900/20 transition-all disabled:opacity-50 disabled:cursor-not-allowed flex items-center gap-2"
                >
                  {loading ? (
                    <>
                      <svg className="animate-spin h-5 w-5 text-white" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                        <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4"></circle>
                        <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                      </svg>
                      <span>Compiling</span>
                    </>
                  ) : (
                    <>
                      <span>Visualize</span>
                      <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M14.752 11.168l-3.197-2.132A1 1 0 0010 9.87v4.263a1 1 0 001.555.832l3.197-2.132a1 1 0 000-1.664z" /><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>
                    </>
                  )}
                </button>
              </div>
            </div>

            {/* Quick Examples */}
            <div className="flex items-center gap-4 text-sm border-t border-white/5 pt-4">
              <span className="text-slate-500 font-medium text-xs uppercase tracking-wider">Presets</span>
              <div className="flex flex-wrap gap-2">
                {mode === 1 ? (
                  <>
                    <button onClick={() => { setInput('[a-zA-Z_][a-zA-Z0-9]*'); setTestString('my_var_1'); }} className="px-3 py-1.5 bg-slate-900 hover:bg-slate-800 text-slate-300 rounded-md border border-white/10 text-xs font-mono transition-colors">Identifier</button>
                    <button onClick={() => { setInput('[0-9]+(\.[0-9]+)?'); setTestString('3.1415'); }} className="px-3 py-1.5 bg-slate-900 hover:bg-slate-800 text-slate-300 rounded-md border border-white/10 text-xs font-mono transition-colors">Float</button>
                    <button onClick={() => { setInput('(a|b)*abb'); setTestString('ababb'); }} className="px-3 py-1.5 bg-slate-900 hover:bg-slate-800 text-slate-300 rounded-md border border-white/10 text-xs font-mono transition-colors">Ends 'abb'</button>
                  </>
                ) : (
                  <>
                    <button onClick={() => setInput('x = 5 + 3')} className="px-3 py-1.5 bg-slate-900 hover:bg-slate-800 text-slate-300 rounded-md border border-white/10 text-xs font-mono transition-colors">Assignment</button>
                    <button onClick={() => setInput('res = ( 10 + 2 ) * 5')} className="px-3 py-1.5 bg-slate-900 hover:bg-slate-800 text-slate-300 rounded-md border border-white/10 text-xs font-mono transition-colors">Complex Math</button>
                  </>
                )}
              </div>
            </div>
          </div>
        </section>

        {/* Dashboards Grid */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 h-[800px]">

          {/* Left: Visualization (8 cols) */}
          <section className="lg:col-span-8 flex flex-col glass-panel rounded-2xl overflow-hidden shadow-2xl shadow-black/50">
            <div className="h-12 bg-[#0A0A0A] border-b border-white/5 flex justify-between items-center px-4">
              <div className="flex items-center gap-2">
                <div className="flex gap-1.5">
                  <div className="w-3 h-3 rounded-full bg-red-500/20 border border-red-500/50"></div>
                  <div className="w-3 h-3 rounded-full bg-amber-500/20 border border-amber-500/50"></div>
                  <div className="w-3 h-3 rounded-full bg-emerald-500/20 border border-emerald-500/50"></div>
                </div>
                <span className="text-xs font-mono text-slate-400 ml-3">
                  {viewMode === 'trace' ? 'Trace Table' : viewMode === 'lexer' ? 'Lexical Tokens' : viewMode.toUpperCase()}
                </span>
              </div>

              {/* Graph Controls */}
              <div className="flex bg-[#050505] p-0.5 rounded-lg border border-white/5">
                {/* Common Tabs */}
                <button onClick={() => setViewMode('trace')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'trace' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>TRACE</button>

                {/* Lexical Tabs */}
                {mode === 1 && graphs.nfa && <button onClick={() => setViewMode('nfa')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'nfa' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>NFA</button>}
                {mode === 1 && graphs.dfa && <button onClick={() => setViewMode('dfa')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'dfa' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>DFA</button>}
                {mode === 1 && graphs.min_dfa && <button onClick={() => setViewMode('min_dfa')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'min_dfa' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>MIN-DFA</button>}

                {/* Syntax Tabs */}
                {mode === 3 && <button onClick={() => setViewMode('lexer')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'lexer' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>TOKENS</button>}
                {mode === 3 && graphs.pda && <button onClick={() => setViewMode('pda')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'pda' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>PDA</button>}
                {mode === 3 && <button onClick={() => setViewMode('grammar')} className={`px-3 py-1 rounded-md text-[10px] font-bold tracking-wider transition-all ${viewMode === 'grammar' ? 'bg-slate-800 text-white shadow-sm' : 'text-slate-500 hover:text-slate-300'}`}>GRAMMAR</button>}
              </div>
            </div>

            {/* Viewer Area */}
            <div className="flex-1 bg-[#050505] relative overflow-hidden flex flex-col">
              <div className="flex-1 relative overflow-auto">
                {/* View Logic */}
                {viewMode === 'trace' ? (
                  <div className="p-4 space-y-1 font-mono text-sm max-h-full">
                    {trace.length > 0 ? (
                      <>
                        {getVisibleStack().map((t, i) => (
                          <div key={i} className="flex gap-4 p-2 rounded hover:bg-white/5 border border-transparent hover:border-white/5 transition-colors group">
                            <div className="w-8 flex-shrink-0 text-slate-600 text-right select-none">{t.data.step}</div>
                            <div className="flex-1 font-mono">
                              <div className="text-slate-400 text-xs mb-0.5 flex justify-between">
                                <span>idx: {t.data.input_index}</span>
                                <span className="text-emerald-500 font-bold">{t.data.action}</span>
                              </div>
                              <div className="text-sky-200">{t.data.stack_content}</div>
                            </div>
                          </div>
                        ))}
                        <div ref={(el) => el?.scrollIntoView({ behavior: 'smooth' })} />
                      </>
                    ) : (
                      <div className="absolute inset-0 flex items-center justify-center text-slate-700">
                        <p className="text-lg font-light tracking-wide">// No trace data available</p>
                      </div>
                    )}
                  </div>
                ) : viewMode === 'lexer' ? (
                  <div className="p-8">
                    <h3 className="text-slate-400 text-xs font-bold uppercase tracking-widest mb-6">Lexical Analysis Token Stream</h3>
                    <div className="flex flex-wrap gap-4">
                      {getTokens().map((t, i) => (
                        <div key={i} className="bg-slate-900 border border-white/10 rounded-xl p-4 min-w-[120px] flex flex-col items-center justify-center shadow-lg group hover:border-sky-500/50 transition-colors">
                          <div className="text-[10px] font-bold text-slate-500 uppercase tracking-wider mb-2">{t.data.token_type}</div>
                          <div className="font-mono text-xl text-sky-400 font-bold">{t.data.value}</div>
                          <div className="mt-2 text-[10px] text-slate-600">ID: {t.data.id}</div>
                        </div>
                      ))}
                    </div>
                    {getTokens().length === 0 && <p className="text-slate-600 italic">No tokens parsed yet.</p>}
                  </div>
                ) : viewMode === 'grammar' ? (
                  <div className="p-8 font-mono">
                    <h3 className="text-slate-400 text-xs font-bold uppercase tracking-widest mb-6">Context-Free Grammar (CFG)</h3>
                    <div className="bg-slate-900/50 p-6 rounded-xl border border-white/5 text-slate-300 space-y-2">
                      <p><span className="text-sky-400">E</span> -> <span className="text-indigo-400">T</span> <span className="text-emerald-400">E'</span></p>
                      <p><span className="text-emerald-400">E'</span> -> + <span className="text-indigo-400">T</span> <span className="text-emerald-400">E'</span> | - <span className="text-indigo-400">T</span> <span className="text-emerald-400">E'</span> | ε</p>
                      <p><span className="text-indigo-400">T</span> -> <span className="text-amber-400">F</span> <span className="text-pink-400">T'</span></p>
                      <p><span className="text-pink-400">T'</span> -> * <span className="text-amber-400">F</span> <span className="text-pink-400">T'</span> | / <span className="text-amber-400">F</span> <span className="text-pink-400">T'</span> | ε</p>
                      <p><span className="text-amber-400">F</span> -> ( <span className="text-sky-400">E</span> ) | id | num</p>
                    </div>
                  </div>
                ) : (
                  <GraphVisualizer
                    dotString={graphs[viewMode]}
                    activeStateId={getActiveStateId()}
                  />
                )}
              </div>

              {/* Playback Grid */}
              <div className="h-16 bg-[#0A0A0A] border-t border-white/5 flex items-center px-4 justify-between">
                <div className="flex items-center gap-2">
                  <button onClick={() => setStep(s => Math.max(-1, s - 1))} disabled={step < 0} className="p-2 rounded hover:bg-white/10 text-slate-400 disabled:opacity-25 transition-colors">
                    <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 24 24"><path d="M15.41 7.41L14 6l-6 6 6 6 1.41-1.41L10.83 12z" /></svg>
                  </button>
                  <button onClick={() => setIsPlaying(!isPlaying)} disabled={trace.length === 0} className="w-10 h-10 rounded-full bg-slate-800 hover:bg-sky-600 text-white flex items-center justify-center transition-all shadow-lg">
                    {isPlaying ? (
                      <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 24 24"><path d="M6 19h4V5H6v14zm8-14v14h4V5h-4z" /></svg>
                    ) : (
                      <svg className="w-4 h-4 ml-0.5" fill="currentColor" viewBox="0 0 24 24"><path d="M8 5v14l11-7z" /></svg>
                    )}
                  </button>
                  <button onClick={() => setStep(s => Math.min(trace.length - 1, s + 1))} disabled={step >= trace.length - 1} className="p-2 rounded hover:bg-white/10 text-slate-400 disabled:opacity-25 transition-colors">
                    <svg className="w-5 h-5" fill="currentColor" viewBox="0 0 24 24"><path d="M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6z" /></svg>
                  </button>
                </div>

                <div className="px-3 py-1 bg-slate-900 rounded border border-white/5 text-xs text-sky-400 font-mono shadow-inner">
                  Step: {step + 1} <span className="text-slate-600">/</span> {trace.length}
                </div>

                <div className="flex items-center gap-3">
                  <span className="text-[10px] text-slate-500 font-bold uppercase">Speed</span>
                  <input type="range" min="100" max="2000" step="100" value={speed} onChange={e => setSpeed(Number(e.target.value))} className="w-24 accent-sky-500 h-1 bg-slate-800 rounded-full appearance-none" />
                </div>
              </div>
            </div>
          </section>

          {/* Right: Logs & History (4 cols) */}
          <section className="lg:col-span-4 flex flex-col gap-6 h-full">

            {/* Terminal Log */}
            <div className="flex-1 glass-panel rounded-2xl overflow-hidden flex flex-col shadow-xl">
              <div className="h-10 bg-[#0A0A0A] border-b border-white/5 flex items-center px-4">
                <span className="text-xs font-semibold text-slate-400 uppercase tracking-widest">System Log</span>
              </div>
              <div className="flex-1 bg-[#020202] p-4 overflow-auto font-mono text-xs leading-relaxed text-slate-300">
                <pre className="whitespace-pre-wrap">
                  {output || <span className="text-slate-700 italic">// Waiting for input...</span>}
                </pre>
              </div>
            </div>

            {/* Recent History */}
            <div className="h-[300px] glass-panel rounded-2xl overflow-hidden flex flex-col shadow-xl">
              <div className="h-10 bg-[#0A0A0A] border-b border-white/5 flex items-center justify-between px-4">
                <span className="text-xs font-semibold text-slate-400 uppercase tracking-widest">History</span>
                <button onClick={fetchHistory} className="text-[10px] text-sky-500 hover:text-sky-400 hover:underline">REFRESH</button>
              </div>
              <div className="flex-1 overflow-auto p-2 space-y-1 bg-[#050505]">
                {history.map(item => (
                  <button
                    key={item.id}
                    onClick={() => {
                      setMode(item.mode);
                      setInput(item.input_text);
                      setTestString(item.test_string || '');
                      if (item.result_output) setOutput(item.result_output);
                    }}
                    className="w-full text-left p-3 rounded-lg hover:bg-white/5 border border-transparent hover:border-white/5 transition-all group"
                  >
                    <div className="flex justify-between items-center mb-1">
                      <span className={`text-[10px] px-1.5 py-0.5 rounded font-bold tracking-wider ${item.mode === 1 ? 'bg-sky-500/10 text-sky-400' : 'bg-indigo-500/10 text-indigo-400'}`}>
                        {item.mode === 1 ? 'REGEX' : 'Syntactic'}
                      </span>
                      <span className="text-[10px] text-slate-600 group-hover:text-slate-500">{new Date(item.created_at).toLocaleTimeString()}</span>
                    </div>
                    <div className="font-mono text-xs text-slate-300 truncate opacity-80 group-hover:opacity-100">{item.input_text}</div>
                    {item.mode === 1 && item.test_string && (
                      <div className="font-mono text-[10px] text-slate-500 truncate mt-0.5">Test: <span className="text-slate-400">{item.test_string}</span></div>
                    )}
                  </button>
                ))}
              </div>
            </div>
          </section>

        </div>
      </main>
    </div>
  );
}

export default App;
