const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

const width = 2000;
const height = 2000;

canvas.width = width; canvas.height = height;

// well have integers in range [0, 255]
const buffer = new SharedArrayBuffer(Uint8Array.BYTES_PER_ELEMENT*width*height);

const grid_size = 300;

const layer_count = 1;

const seed = Math.random() * 10000;

var time;

const numWorkers = navigator.hardwareConcurrency || 4; // Use number of CPU cores or default to 4

const chunkWidth = width / numWorkers;

parallelDraw();

// divide noise generation into numWorkers chunk count
function parallelDraw () {

    var workersDone = 0;
    const workers = [];

    time = performance.now();

    for (let i=0; i < numWorkers; i++) {
        const worker = new Worker('./worker.js');
        workers.push(worker);

        const startX = i * chunkWidth;
        const endX = Math.min(startX + chunkWidth, width);

        // define onmessage
        worker.onmessage = function(e) {
            if (e.data.status === "done") {
                workersDone++;
                if (workersDone === numWorkers) {
                    time = performance.now() - time;
                    console.log(time, "milliseconds, with ", numWorkers, "workers");
                    toCanvas();
                    workers.forEach(w => w.terminate());
                }
            }
        };

        // define onerror
        worker.onerror = function(err) {
            console.log(`worker error: ${err.message}`);
            workersDone++;
            if (workersDone === numWorkers) {
                time = performance.now() - time;
                console.log(time, "milliseconds, with ", numWorkers, "workers");
                toCanvas();
                workers.forEach(w => w.terminate());
            }
        };

        worker.postMessage({ startX, endX, width, height, grid_size, layer_count, seed, buffer });
    }
}

function toCanvas () {

    const perlinMatrix = new Uint8Array(buffer);
    var id = ctx.createImageData(canvas.width, canvas.height);
    var d = id.data;

    for (let x=0; x < canvas.height; x++) {
        for (let y=0; y < canvas.width; y++) {

            const m = perlinMatrix[x * width + y];
            const index = (x * width + y) * 4;
            d[index] = m; d[index+1] = m; d[index+2] = m; d[index+3] = 255;
        }
    }

    ctx.putImageData(id, 0, 0);

    var url = canvas.toDataURL('image/png');

    var link = document.createElement('a');
    link.href = url;
    link.download = 'perlin_noise.png';
    link.text = "download image"

    document.body.appendChild(link);
}