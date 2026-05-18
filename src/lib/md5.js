import bindings from 'bindings';
import crypto from 'crypto';
import fs from 'fs';

const md5 = bindings('md5_v2');

const toBin32 = (num) => {
    return (num >>> 0).toString(2).padStart(32, '0').replace(/(.{8})/g, '$1 ');
};

// Helper to convert numbers to fixed-width Hex
const toHex32 = (num) => {
    return '0x' + (num >>> 0).toString(16).toUpperCase().padStart(8, '0');
};

function captureStep(steps) {
    return function (step) {
        steps.push({
            blk_index: step.blk_index,
            block: step.block,
            round: step.round,
            a: step.a,
            b: step.b,
            c: step.c,
            d: step.d,
            temp: step.temp
        });
    }
}

async function getSteps(input) {
    console.log("Processing MD5 in worker thread...");
    const steps = []
    const result = await md5.computeMD5(input, captureStep(steps));

    console.log(`MD5 Result: ${result.toString('hex')}`);
    console.log(`Captured ${steps.length} total rounds. Generating HTML...`);
    return {result, steps}
}

async function run() {
    const input1 = 0b10000000
    const res1 = await getSteps(Buffer.from([input1]))
    const input2 = 0b10000001
    const res2 = await getSteps(Buffer.from([input2]))

    const trackList = ['a', 'b'];

    const analyticalPayload = {
        [`Input_${toBin32(input1)}`]: {
            block: 0,
            steps: res1.steps,
            result: res1.result,
        },
        [`Input_${toBin32(input2)}`]: {
            block: 0,
            steps: res2.steps,
            result: res2.result,
        }
    }

    const finalHTMLOutput = generateComparisonHTML(analyticalPayload, trackList);
    fs.writeFileSync('md5_dynamic_comparison.html', finalHTMLOutput);
    console.log("Analysis saved to 'md5_analysis.html'. Open this file in any browser!");
}

function formatBinaryBlock(buffer) {
    if (!buffer || !(Buffer.isBuffer(buffer) || buffer instanceof Uint8Array)) {
        return "No raw buffer captured.";
    }
    return Array.from(buffer).map((byte, idx) => {
        return byte.toString(2).padStart(8, '0') + (((idx + 1) % 8 === 0) ? '\n' : ' ');
    }).join('').trim();
}

function extractCoordinates(steps, targetBlock, variableKey) {
    // Match against your native struct property naming (blk_index)
    const filtered = steps.filter(d => d.blk_index === targetBlock);

    const points = [];
    const meta = {};
    let minY = null;
    let maxY = null;
    let rawBlockBuffer = null;

    if (filtered.length > 0 && filtered[0].block) {
        rawBlockBuffer = filtered[0].block;
    }

    filtered.forEach(d => {
        const val = d[variableKey];
        points.push({ x: d.round, y: val });

        if (minY === null || val < minY) minY = val;
        if (maxY === null || val > maxY) maxY = val;

        meta[d.round] = { hex: toHex32(val), bin: toBin32(val) };
    });

    return { points, meta, minY, maxY, rawBlockBuffer };
}

/**
 * Function 2: Generates a complete diagnostic report comparing two separate datasets
 */
// datasets -> { `$dataset_id`: {steps: [...], block: number}, `$dataset_id`: {steps: [...], block: number}
function generateComparisonHTML(datasets, variablesToCompare) {
    let compiledSections = "";

    // Vibrant colors matching dark mode presentation
    const colorPalette = ['#00adb5', '#ff2e63', '#ffde7d', '#4e9f3d', '#9a52c7'];
    const datasetIds = Object.keys(datasets);
    let hashSummaryRowsHTML = "";
    datasetIds.forEach((id, index) => {
        const dataset = datasets[id]
        const activeColor = colorPalette[index % colorPalette.length];
        hashSummaryRowsHTML += `
            <div class="hash-tag-box" style="border-left: 4px solid ${activeColor}">
                <strong>${id}:</strong>
                <div>
                    Hex: <span class="hash-tag" style="color: ${activeColor}">${dataset.result.toString('hex')}</span>
                    Bin: <span class="hash-tag" style="color: ${activeColor}">${formatBinaryBlock(dataset.result)}</span>
                </div>
            </div>
        `;
    })

    // Loop through each requested internal field variable target
    variablesToCompare.forEach(varKey => {
        const chartDatasetsConfig = [];
        const clientMetaCollection = [];
        let binaryBlockLayoutsHTML = "";

        let globalMinY = null;
        let globalMaxY = null;

        // Process all active input data sets sequentially
        datasetIds.forEach((id, index) => {
            const currentRun = datasets[id];
            const extracted = extractCoordinates(currentRun.steps, currentRun.block, varKey);

            // Compute uniform visualization bounding scales
            if (extracted.minY !== null && (globalMinY === null || extracted.minY < globalMinY)) globalMinY = extracted.minY;
            if (extracted.maxY !== null && (globalMaxY === null || extracted.maxY > globalMaxY)) globalMaxY = extracted.maxY;

            // Append ChartJS structural data mappings
            chartDatasetsConfig.push({
                label: `${id} (Block ${currentRun.block}): ${varKey.toUpperCase()}`,
                data: extracted.points,
                backgroundColor: colorPalette[index % colorPalette.length],
                pointRadius: datasetIds.length > 2 ? 4 : 6,
                pointHoverRadius: 8
            });

            // Store matching meta lookups to index against the internal ChartJS dataset layout
            clientMetaCollection.push(extracted.meta);

            // Build side-by-side matrix dump structures
            binaryBlockLayoutsHTML += `
            <div class="buffer-sub-panel">
                <strong>[${id}] Block ${currentRun.block} Buffer (512-bit):</strong>
                <pre class="binary-block-display">${formatBinaryBlock(extracted.rawBlockBuffer)}</pre>
            </div>
            `;
        });

        const uniqueId = `chart_compare_${varKey}`;

        // Inject the isolated graph block section wrapper component context component logic
        compiledSections += `
        <div class="block-card">
            <div class="meta-panel">
                <h2>[Avalanche Trace: Register ${varKey.toUpperCase()}]</h2>
                <div class="buffer-layouts-container">
                    ${binaryBlockLayoutsHTML}
                </div>
            </div>

            <div class="chart-container">
                <canvas id="${uniqueId}"></canvas>
            </div>
        </div>

        <script>
        (function() {
            const ctx = document.getElementById('${uniqueId}').getContext('2d');
            const runtimeMetaCollection = ${JSON.stringify(clientMetaCollection)};

            new Chart(ctx, {
                type: 'scatter',
                data: {
                    datasets: ${JSON.stringify(chartDatasetsConfig)}
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        x: {
                            type: 'linear',
                            position: 'bottom',
                            title: { display: true, text: 'Rounds (0 to 63)', color: '#fff' },
                            min: 0,
                            max: 63,
                            ticks: { stepSize: 1, color: '#aaa' },
                            grid: { color: '#444' }
                        },
                        y: {
                            title: { display: true, text: '32-Bit Unsigned Matrix Value', color: '#fff' },
                            grid: { color: '#444' },
                            min: ${globalMinY},
                            max: ${globalMaxY},
                            ticks: {
                                color: '#aaa',
                                callback: function(value) {
                                    return '0x' + (value >>> 0).toString(16).toUpperCase();
                                }
                            }
                        }
                    },
                    plugins: {
                        legend: { labels: { color: '#fff' } },
                        tooltip: {
                            backgroundColor: '#333',
                            titleColor: '#fff',
                            borderColor: '#555',
                            borderWidth: 1,
                            callbacks: {
                                title: function(context) {
                                    return 'Round ' + context[0].raw.x;
                                },
                                label: function(context) {
                                    const round = context.raw.x;
                                    const datasetIndex = context.datasetIndex; // Maps directly back to loop creation index order
                                    const meta = runtimeMetaCollection[datasetIndex][round];
                                    return [
                                        context.dataset.label + ' Details:',
                                        '  Hex: ' + meta.hex,
                                        '  Bin: ' + meta.bin
                                    ];
                                }
                            }
                        }
                    }
                }
            });
        })();
        </script>
        <hr class="section-divider" />
        `;
    });

    return `
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MD5 Multi-Run Variable Comparative Inspection Matrix</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; margin: 30px; background: #1a1a1a; color: #e0e0e0; }
        .container { max-width: 1400px; margin: 0 auto; background: #2d2d2d; padding: 30px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); }
        h1, h2 { color: #fff; margin-bottom: 5px; }
        .global-header { border-bottom: 2px solid #444; padding-bottom: 15px; margin-bottom: 30px; }
        .block-card { background: #222; padding: 20px; border-radius: 8px; margin-bottom: 25px; }
        .meta-panel { background: #1a1a1a; border-left: 4px solid #ffde7d; padding: 15px; margin-bottom: 20px; border-radius: 4px; font-family: monospace; }
        
        /* Auto-adjust column panels dynamically based on how many runs are active simultaneously */
        .buffer-layouts-container { display: flex; flex-direction: row; gap: 20px; flex-wrap: wrap; }
        .buffer-sub-panel { flex: 1; min-width: 280px; }
        
        .binary-block-display { background: #000; color: #39ff14; padding: 12px; border-radius: 4px; overflow-x: auto; white-space: pre-wrap; font-size: 0.8em; line-height: 1.4; border: 1px solid #333; margin-top: 6px; }
        .chart-container { position: relative; height: 480px; width: 100%; background: #111; padding: 10px; border-radius: 8px; border: 1px solid #333; }
        .section-divider { border: 0; height: 1px; background-image: linear-gradient(to right, rgba(255,255,255,0), rgba(255,255,255,0.15), rgba(255,255,255,0)); margin: 40px 0; }
    </style>
</head>
<body>
    <div class="container">
        <div class="global-header">
            <h1>MD5 Advanced Avalanche Verification Matrix</h1>
            <p>Final Hash Matrix Values:</p>
            <!-- Dynamic Injection Node for loop output -->
            ${hashSummaryRowsHTML}
        </div>
        ${compiledSections}
    </div>
</body>
</html>
    `;
}

run();