import { useEffect, useState, useRef } from 'react';
import { Graphviz } from 'graphviz-react';

const GraphVisualizer = ({ dotString, activeStateId }) => {
    const [processedDot, setProcessedDot] = useState(dotString);
    const containerRef = useRef(null);
    const prevActiveStateIdRef = useRef(null);

    useEffect(() => {
        if (!dotString) return;

        let newDot = dotString;

        // Highlight active state
        if (activeStateId !== null && activeStateId !== undefined) {
            // Extract just the ID number if it comes in as "q0" or "0"
            // Our ID is usually just the number string "0", "1" etc.
            // But the DOT file uses labels like "q0" or "0".
            // We need to match what's in the DOT file.

            // Try matching "qX" first (NFA/DFA), then just "X" (PDA)
            let stateLabel = `q${activeStateId}`;
            // If the dot string doesn't contain "qX", try just "X"
            if (!dotString.includes(`  ${stateLabel} [`)) {
                stateLabel = `${activeStateId}`;
            }

            const target = `  ${stateLabel} [`;
            const replacement = `  ${stateLabel} [style="filled", fillcolor="#0ea5e9", fontcolor="white", `;

            if (newDot.includes(target)) {
                newDot = newDot.replace(target, replacement);
            } else {
                const styleLine = `  ${stateLabel} [style="filled", fillcolor="#0ea5e9", fontcolor="white"];\n`;
                newDot = newDot.replace('}', styleLine + '}');
            }
        }

        setProcessedDot(newDot);
    }, [dotString, activeStateId]);

    // Animation Effect
    useEffect(() => {
        if (!containerRef.current || activeStateId === null || prevActiveStateIdRef.current === null) {
            prevActiveStateIdRef.current = activeStateId;
            return;
        }

        const fromId = prevActiveStateIdRef.current;
        const toId = activeStateId;

        // Safety check
        if (typeof window === 'undefined') return;

        try {
            // Don't animate if staying on same node (unless self-loop, but checking logic is complex)
            if (fromId === toId) return;

            // Find the SVG in the DOM
            const svg = containerRef.current.querySelector('svg');
            if (!svg) return;

            // Graphviz generates edges result in <g class="edge"> with <title>q0->q1</title> or similar
            // We need to find the edge connecting these two.
            // Title format is usually "NodeA->NodeB"

            // Try formats: "q0->q1" or "0->1"
            const possibleTitles = [
                `q${fromId}->q${toId}`,
                `${fromId}->${toId}`,
                `q${fromId}->${toId}`, // Mixed (unlikely)
                `${fromId}->q${toId}`  // Mixed (unlikely)
            ];

            let edgePath = null;
            const edges = svg.querySelectorAll('.edge');

            for (const edge of edges) {
                const title = edge.querySelector('title');
                if (title && possibleTitles.includes(title.textContent)) {
                    edgePath = edge.querySelector('path');
                    break;
                }
            }

            if (edgePath) {
                // Create a particle
                const particle = document.createElementNS("http://www.w3.org/2000/svg", "circle");
                particle.setAttribute("r", "4");
                particle.setAttribute("fill", "#38bdf8"); // Sky-400
                particle.setAttribute("filter", "drop-shadow(0 0 2px #38bdf8)");

                // Append to SVG (ensure it's on top)
                svg.appendChild(particle);

                // Animate
                try {
                    // Get the path data
                    const pathData = edgePath.getAttribute('d');

                    // Check if offset-path is supported (it is in Chrome/Edge, but maybe not all)
                    if (particle.style.offsetPath !== undefined) {
                        particle.style.offsetPath = `path('${pathData}')`;

                        const animation = particle.animate([
                            { offsetDistance: '0%' },
                            { offsetDistance: '100%' }
                        ], {
                            duration: 400, // ms
                            easing: 'ease-in-out',
                            fill: 'forwards'
                        });

                        animation.onfinish = () => {
                            if (particle.parentNode) particle.parentNode.removeChild(particle);
                        };
                    } else {
                        // Fallback: just remove it if animation API not supported fully
                        if (particle.parentNode) particle.parentNode.removeChild(particle);
                    }
                } catch (animErr) {
                    // console.warn("Animation failed", animErr);
                    if (particle.parentNode) particle.parentNode.removeChild(particle);
                }
            }
        } catch (e) {
            console.error("Graph animation error:", e);
        }

        prevActiveStateIdRef.current = activeStateId;

    }, [activeStateId, processedDot]); // processedDot dependency ensures we re-bind if graph changes

    if (!dotString) return <div className="text-slate-500 italic p-4">No graph data available.</div>;

    return (
        <div ref={containerRef} className="graph-container overflow-auto bg-[#0B0F19] p-4 rounded-xl border border-slate-800 h-full flex items-center justify-center">
            <Graphviz dot={processedDot} options={{ height: "100%", width: "100%", fit: true, zoom: true }} />
        </div>
    );
};

export default GraphVisualizer;
