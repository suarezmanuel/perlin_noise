createModule().then(Module => {
    console.log('WASM Module initialized');

    const width = 1000;  // Desired width
    const height = 1000; // Desired height
    const imageDataSize = width * height;

    let yOffset = 0;

    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('2d');
    canvas.width = width;
    canvas.height = height;

    const imageData = ctx.createImageData(width, height);
    const d = imageData.data;

    const genNoise = Module.cwrap('generateNoise', null, ['number', 'number', 'number', 'number', 'number', 'number']);
    const my_malloc = Module.cwrap('my_malloc', 'number', ['number']);
    const my_free = Module.cwrap('my_free', null, ['number']);

    const noiseDataPtr = my_malloc(imageDataSize);

    if (!noiseDataPtr) {
        console.error('Failed to allocate memory for noise data.');
        return;
    }

    const noiseDataHeap = new Uint8Array(Module.HEAPU8.buffer, noiseDataPtr, imageDataSize);

    function update() {
        genNoise(
            0,            // startX
            yOffset,      // startY
            0,            // startZ
            width,
            height,
            noiseDataPtr  // buffer pointer
        );

        yOffset++;

        for (let i = 0, j = 0; i < imageDataSize; i++, j += 4) {
            const m = noiseDataHeap[i];
            d[j] = m;     // Red channel
            d[j + 1] = m; // Green channel
            d[j + 2] = m; // Blue channel
            d[j + 3] = 255; // Alpha channel
        }

        ctx.putImageData(imageData, 0, 0);

        requestAnimationFrame(update);
    }

    requestAnimationFrame(update);

    window.addEventListener('beforeunload', () => {
        my_free(noiseDataPtr);
    });

}).catch(err => {
    console.error('Error initializing WASM Module:', err);
});
