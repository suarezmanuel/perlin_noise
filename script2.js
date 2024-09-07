const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
canvas.width = 800;
canvas.height = 800;

const layer_count = 7;

const grid_size = 70;

const rand_pseudo_seed = Math.random() * 10000;

window.requestAnimationFrame(draw);


function draw () {

    var pixelData = new Uint8ClampedArray(canvas.width*canvas.height*4);
    let index = 0;
    let c = 255*0.5;

    var start = performance.now();

    for (let x=0; x < canvas.width; x++) {
        for (let y=0; y < canvas.height; y++) {

            let amp = 1;
            let freq = 1;
            let val = 0;
            
            for (let i=0; i < layer_count; i++) {
                val += perlin(x*freq/grid_size ,y*freq/grid_size) * amp;
                freq *= 2;
                amp /= 2;
            }

            val *= 1.2
            val = Math.min(1, Math.max(-1, val));

            // convert range [-1, 1] to [0, 255]
            let color = (val+1) * c;
            pixelData[index++] = color;
            pixelData[index++] = color;
            pixelData[index++] = color;
            pixelData[index++] = 255;
        }
    }

    var id = ctx.createImageData(canvas.width, canvas.height);
    id.data.set(pixelData);
    ctx.putImageData(id,0,0);

    console.log(performance.now() - start);

    canvas_to_image(canvas);
}

function canvas_to_image () {
    var url = canvas.toDataURL('image/png');

    var link = document.createElement('a');
    link.href = url;
    link.download = 'perlin_noise.png';
    link.text = "download image"
    // link.width = 10;

    document.body.appendChild(link);
}

function perlin (x, y) {

    let x0 = Math.floor(x);
    let x1 = Math.ceil(x);
    let y0 = Math.floor(y);
    let y1 = Math.ceil(y);

    let v00 = [x0-x, y0-y];
    let v10 = [x1-x, y0-y];
    let v01 = [x0-x, y1-y];
    let v11 = [x1-x, y1-y];

    let t1 = cubic_interpolate(dot_product(v00, get_random_vec(x0,y0)), dot_product(v10, get_random_vec(x1,y0)), x-x0);
    let t2 = cubic_interpolate(dot_product(v01, get_random_vec(x0,y1)), dot_product(v11, get_random_vec(x1,y1)), x-x0);

    return cubic_interpolate(t1, t2, y-y0);
}

// we want the same points to get the same randoms
// other than that, its completely random
function get_random_vec (x, y) {
    const seed = rand_pseudo_seed + x*3284157443 + rand_pseudo_seed + y*1911520717;
    const angle = seeded_random(seed) * 2*Math.PI;
    return [Math.cos(angle), Math.sin(angle)];
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