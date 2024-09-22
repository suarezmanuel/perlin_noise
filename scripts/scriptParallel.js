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

const vectors = await getVectors();

async function getVectors() {
    const response = await fetch("../resources/vectorsGood.txt");
    const text = await response.text();
    return JSON.parse(text);
}

parallelDraw();

// divide noise generation into numWorkers chunk count
function parallelDraw () {

    var workersDone = 0;
    const workers = [];

    time = performance.now();

    for (let i=0; i < numWorkers; i++) {
        const worker = new Worker('./scripts/worker.js');
        workers.push(worker);

        const startX = i * chunkWidth;
        const endX = Math.min(startX + chunkWidth, width);

        // define onmessage
        worker.onmessage = function(e) {
            if (e.data.status === "done") {
                workersDone++;
                if (workersDone === numWorkers) done(time, workers);
            }
        };

        // define onerror
        worker.onerror = function(err) {
            console.log(`worker error: ${err.message}`);
            workersDone++;
            if (workersDone === numWorkers) done(time, workers);
        };

        worker.postMessage({ startX, endX, width, height, grid_size, layer_count, seed, buffer, vectorsGiven:vectors });
    }
}

function done(startTime, workers) {
    console.log(performance.now() - startTime, "milliseconds, with ", numWorkers, "workers");
    toCanvas();
    workers.forEach(w => w.terminate());
    plotProbs(getProbs(0,0,width/2,height/2));
    plotProbs(getProbs(width/2,height/2,width,height));
    plotCorr(getProbs(0,0,width/2,height/2), getProbs(width/2,height/2,width,height))
    shannonEntropy();
}

function getProbs(startX, startY, width, height) {
    const perlinMatrix = new Uint8Array(buffer);
    var probs = new Array(256).fill(0);
    for (let i=startY; i<startY+height; i++) {
        for (let j=startX; j<startX+width; j++) {
            const value = perlinMatrix[i*width + j];
            probs[value]++;
        }
    }
    return probs;
}

function shannonEntropy() {

    var probs = getProbs(0,0,width,height);

    var entropy = 0;
    for (let i=0; i<256; i++) {
        // divide by amount of samples to get a probability
        const prob = probs[i]/(width*height);
        if (prob == 0) continue;
        entropy -= prob*Math.log2(prob);
    }
    // values are of 8 bits, so closer to 8 means each bit is random
    console.log("shannon entropy, closer to 8 better", entropy);
}

function plotProbs(probs) {
    const statCanvas = document.createElement('canvas');
    statCanvas.width = 255*2;
    statCanvas.height = 255*2;
    const statCtx = statCanvas.getContext('2d');

    const maxProb = Math.max(...probs);
    const scaleY = maxProb == 0 ? 1 : statCanvas.height/maxProb;

    for (let i=0; i<probs.length; i++) {
        const c = Math.trunc(probs[i]);
        statCtx.fillStyle = `rgb(${i},${i},${i})`
        statCtx.beginPath();
        statCtx.arc(i*2, statCanvas.height-c*scaleY, 4, 0, 2*Math.PI);
        statCtx.fill();
    }

    document.body.appendChild(statCanvas);
}

// assuming they operate over the same supp
function plotCorr(probs1, probs2) {

     // overshadows the public canvas
    const canvas = document.createElement('canvas');
    canvas.width = 255*2; canvas.height = 255*2;

    // overshadows the public ctx
    const ctx = canvas.getContext('2d');

    const maxProbX = Math.max(...probs1);
    const maxProbY = Math.max(...probs2);
    const scaleX = maxProbX == 0 ? 1 : canvas.height/maxProbX;
    const scaleY = maxProbY == 0 ? 1 : canvas.width/maxProbY;

    for (let i=0; i<probs1.length; i++) {
        const c1 = Math.trunc(probs1[i]);
        const c2 = Math.trunc(probs2[i]);
        ctx.fillStyle = `rgb(${i},${i},${i})`
        ctx.beginPath();
        ctx.arc(c1*scaleX, canvas.height-c2*scaleY, 4, 0, 2*Math.PI);
        ctx.fill();
    }

    document.body.appendChild(canvas);
}

function autoCoorelation() {

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