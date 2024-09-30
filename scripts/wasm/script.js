createModule().then(Module => {
    console.log('WASM Module initialized');

    const width = 3000; // Your desired width
    const height = 3000; // Your desired height
    const grid_size = 300;
    const layer_count = 1;
    const seed = Math.random() * 10000;

    // Load vectors
    // fetch("../../resources/vectorsGood.txt")
    //     .then(response => response.text())
    //     .then(text => {
            // const vectors = JSON.parse(text);

            // Call the generateNoise function
            let timeToCalc = performance.now();
            // const noisePtr = Module.generateNoise(
            //     width,
            //     height,
            //     layer_count,
            //     grid_size,
            //     seed
            // );

            const noisePtr = Module.generateNoise(
                0,
                0,
                width,
                height
            );
    
            timeToCalc = performance.now() - timeToCalc;

            // Create a typed array view over the WASM memory (shared buffer)
            const imageDataSize = width * height;
            const sharedNoiseData = new Uint8Array(Module.HEAPU8.buffer, noisePtr, imageDataSize);

            // Copy the data into a new non-shared Uint8ClampedArray
            const noiseData = new Uint8ClampedArray(imageDataSize);
            noiseData.set(sharedNoiseData);

            // Render the image onto the canvas
            const canvas = document.getElementById('canvas');
            const ctx = canvas.getContext('2d');
            canvas.width = width;
            canvas.height = height;

            const imageData = ctx.createImageData(width, height);
            const d = imageData.data;

            let timeToRender = performance.now();
            for (let y = 0; y < height; y++) {
                for (let x = 0; x < width; x++) {
                    const m = noiseData[y * width + x];
                    const index = (y * width + x) * 4;
                    d[index] = m;
                    d[index + 1] = m;
                    d[index + 2] = m;
                    d[index + 3] = 255;
                }
            }
            timeToRender = performance.now() - timeToRender;

            ctx.putImageData(imageData, 0, 0);

            var span = document.createElement('span');

            span.innerHTML = `time to calculate: ${timeToCalc.toFixed(2)} ms<br>
                                time to render: ${timeToRender.toFixed(2)} ms`;

            span.style.fontSize = `50px`;

            let div = document.getElementById('content');
            div.style.width = canvas.width;
            div.appendChild(span);
            // Free the noise data in WASM memory
            // Module.freeNoise(noisePtr);
        // })
        // .catch(err => {
        //     console.error('Error loading vectors:', err);
        // });
}).catch(err => {
    console.error('Error initializing WASM Module:', err);
});
