importScripts('./workerWasm.js');

let Module;
let moduleReady = false;
let messageQueue = [];

// Set the onmessage handler immediately
onmessage = function(e) {
    if (moduleReady) {
        processMessage(e);
    } else {
        // Queue the message until the module is ready
        messageQueue.push(e);
    }
};

createModule().then((instance) => {
    Module = instance;
    moduleReady = true;
    console.log('WASM Module initialized');

    // Process any messages that were queued before the module was ready
    while (messageQueue.length > 0) {
        const queuedMessage = messageQueue.shift();
        processMessage(queuedMessage);
    }

}).catch((err) => {
    console.error('Error initializing WASM Module:', err);
});

// Define the message processing function
function processMessage(e) {
    const { startX, endX, width, height, grid_size, layer_count, seed, buffer, vectorsGiven } = e.data;

    // Create a Uint8Array view of the SharedArrayBuffer
    const bufferArray = new Uint8Array(buffer);

    // Call the generatePerlinMatrix function
    Module.generatePerlinMatrix(
        startX,
        endX,
        width,
        height,
        grid_size,
        layer_count,
        seed,
        bufferArray,
        vectorsGiven
    );

    // Notify the main thread that processing is done
    postMessage({ status: 'done' });
}
