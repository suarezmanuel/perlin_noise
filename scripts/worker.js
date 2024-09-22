var vectors = [];

onmessage = function (e) {
    const { startX, endX, width, height, grid_size, layer_count, seed, buffer, vectorsGiven } = e.data;
    vectors = vectorsGiven;
    const perlinMatrix = new Uint8Array(buffer);

    for (let x=0; x < height; x++) {
        for (let y=startX; y < endX; y++) {

            let amp = 1;
            let freq = 1;
            let val = 0;
            
            for (let i=0; i < layer_count; i++) {
                val += perlin(x*freq/grid_size ,y*freq/grid_size, seed) * amp;
                freq *= 2;
                amp /= 2;
            }

            val *= 1.2
            val = Math.min(1, Math.max(-1, val));
            // convert range [-1, 1] to [0, 255], 127.5 = 255/2
            perlinMatrix[x * width + y] = Math.floor((val+1) * 127.5);
        }
    }

    postMessage({ status: 'done' });
}

function perlin (x, y, seed) {

    let x0 = Math.floor(x);
    let x1 = Math.ceil(x);
    let y0 = Math.floor(y);
    let y1 = Math.ceil(y);

    let v00 = [x0-x, y0-y];
    let v10 = [x1-x, y0-y];
    let v01 = [x0-x, y1-y];
    let v11 = [x1-x, y1-y];

    let t1 = cubic_interpolate(dot_product(v00, get_random_vec(x0,y0,seed)), dot_product(v10, get_random_vec(x1,y0,seed)), x-x0);
    let t2 = cubic_interpolate(dot_product(v01, get_random_vec(x0,y1,seed)), dot_product(v11, get_random_vec(x1,y1,seed)), x-x0);

    return cubic_interpolate(t1, t2, y-y0);
}

// we want the same points to get the same randoms
// other than that, its completely random
function get_random_vec (x, y, seed) {
    const hash = Math.floor(seed + x*3284157443 + seed + y*1911520717);
    return vectors[hash % vectors.length];
    // const angle = seeded_random(hash) * 2*Math.PI;
    // return [Math.cos(angle), Math.sin(angle)];


}

function seeded_random (seed) {
    let x = Math.sin(seed) * 10000;
    return  x - Math.floor(x);
}

// imagine being on the unit circle
function get_unit_vector (theta) {
    let x = Math.cos(theta); 
    let y = Math.sin(theta);
    return [x,y];
}

// dot product = inner product in R^2
function dot_product (v, u) {
    return v[0]*u[0] + v[1]*u[1];
}

// smooth step
function cubic_interpolate (a, b, w) {
    return (b - a) * (3.0 - w * 2.0) * w * w + a;
}