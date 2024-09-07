const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
canvas.width = 800;
canvas.height = 800;
// grid will be a square, size in cells
const init_grid_size = 10;
const grid_size = 2*init_grid_size-2;
// horizontal size
const init_cell_size = canvas.width / init_grid_size;
const cell_size = canvas.width / grid_size;
// cell count in width
const cell_count = (canvas.width / cell_size)-3;
const layer_count = 1;

const off = 50;

let init_vec_matrix = [];
let vec_matrix = [];


window.requestAnimationFrame(animate);


function animate () {

    init_vectors();
    // console.log(init_vec_matrix);
    // draw_vectors(ctx, off, init_grid_size, init_cell_size, init_vec_matrix);

    init_perlin_vectors();

    draw_vectors(ctx, off + cell_size/2, grid_size-3, cell_size, vec_matrix);

    perlin_all (layer_count, canvas.width / init_cell_size, init_cell_size, init_vec_matrix);

    // perlin_all (layer_count, grid_size, cell_size, vec_matrix);

}

function perlin_all (layer_count, cell_count, cell_size, vec_matrix) {

    var id = ctx.createImageData(1,1);
    var d = id.data;

    let c = 255*0.5;

    let X = cell_count*cell_size - 2*cell_size;

    for (let x=0; x < X; x++) {
        for (let y=0; y < X; y++) {

            let amp = 1;
            let freq = 1;
            let val = 0;
            let p = 0;
            console.log(y);
            
            for (let i=0; i < layer_count; i++) {
                val += perlin(x*freq, y*freq, cell_size, vec_matrix) * amp;
                freq *= 2;
                amp /= 2;
            }

            // val *= 1.2;
            // if (val > 1) {val = 1}
            // if (val < -1) {val = -1}

            // convert range [-1, 1] to [0, 255]
            d[0] = (val+1) * c;
            d[1] = d[0];
            d[2] = d[0];
            d[3] = 255;
            ctx.putImageData(id, x, y);
        }
    }
}

// a perlin noiser
function perlin (x, y, cell_size, vec_matrix) {

    // console.log(x, y);

    // top left corner
    let tl_i = Math.floor(y / cell_size);
    let tl_j = Math.floor(x / cell_size);

    console.log("i: " + tl_i, "j: " + tl_j);
    // console.log(tl_j);

    let tj = (tl_j*cell_size - x) / cell_size;
    let ti = (tl_i*cell_size - y) / cell_size;
    // normalized distance vectors from point to cell corners
    // let v_tl = [(tl_j*cell_size - x)     / cell_size, (tl_i*cell_size - y) / cell_size];
    // let v_tr = [((tl_j+1)*cell_size - x) / cell_size, (tl_i*cell_size - y) / cell_size];
    // let v_bl = [(tl_j*cell_size - x)     / cell_size, ((tl_i+1)*cell_size - y) / cell_size];
    // let v_br = [((tl_j+1)*cell_size - x) / cell_size, ((tl_i+1)*cell_size - y) / cell_size];


    let v_tl = [tj, ti];
    let v_tr = [tj+1, ti];
    let v_bl = [tj, ti+1];
    let v_br = [tj+1, ti+1];

    // console.log(v_tl);
    // console.log(v_tr);
    // console.log(v_bl);
    // console.log(v_br);

    // normalized distance from left cell wall, x coord
    // is positive
    // let dx = -tj;
    // let dy = -ti;

    let i1 = cubic_interpolate(dot_product(v_tl, vec_matrix[tl_i][tl_j]),
                               dot_product(v_tr, vec_matrix[tl_i][tl_j+1]),
                               -tj);
    
    // console.log(vec_matrix[tl_i][tl_j]);
    // console.log(vec_matrix[tl_i][tl_j+1]);
    // console.log(vec_matrix[tl_i+1][tl_j]);
    // console.log(vec_matrix[tl_i+1][tl_j+1]);

    let i2 = cubic_interpolate(dot_product(v_bl, vec_matrix[tl_i+1][tl_j]),
                               dot_product(v_br, vec_matrix[tl_i+1][tl_j+1]),
                               -tj);

    return cubic_interpolate(i1, i2, -ti);
}

// a grid drawer
function draw_grid (ctx) {

    for (let i=0; i <= grid_size; i++) {
        ctx.beginPath();
        ctx.moveTo(0, cell_size*i);
        ctx.lineTo(canvas.width, cell_size*i);
        ctx.stroke();
    }

    for (let i=0; i <= grid_size; i++) {
        ctx.beginPath();
        ctx.moveTo(cell_size*i, 0);
        ctx.lineTo(cell_size*i, canvas.height);
        ctx.stroke();
    }
}

function init_vectors () {

    for (let i=0; i <= init_grid_size; i++) {
        init_vec_matrix.push([]);
        for (let j=0; j <= init_grid_size; j++) {
            let v = get_unit_vector(Math.random() * 2*Math.PI);
            init_vec_matrix[i].push(v);
        }
    }
}

function init_perlin_vectors () {
        // let grid_size = 2*init_cell_size;
    for (let i=0; i < grid_size; i++) {
        vec_matrix.push([]);
        for (let j=0; j < grid_size; j++) {

            // cell_size/2 = init_cell_size/4

            let theta = (perlin(cell_size/2 + i*cell_size, cell_size/2 + j*cell_size, init_cell_size, init_vec_matrix)+1)*Math.PI;
            vec_matrix[i].push(get_unit_vector(theta));
            console.log(Math.sqrt(Math.pow(vec_matrix[i][j][0],2) + Math.pow(vec_matrix[i][j][1],2)));
            
            // ctx.beginPath();
            // ctx.moveTo(off+cell_size/2 + i*cell_size, off+cell_size/2 + j*cell_size);
            // ctx.lineTo(off+cell_size/2 + i*cell_size + vec_matrix[i][j][0],
            //            off+cell_size/2 + j*cell_size + vec_matrix[i][j][1]);
            // ctx.stroke();
        }
    }
}

function draw_vectors (ctx, off, grid_size, cell_size, vec_matrix) {
    
    const increment = 10;
    for (let i=0; i <= grid_size; i++) {
        for (let j=0; j <= grid_size; j++) {

            // console.log(init_vec_matrix[i][j]);
            ctx.beginPath();
    
            ctx.moveTo(off+i*cell_size, off+j*cell_size);
            ctx.lineTo(off+i*cell_size + vec_matrix[i][j][0]*increment,
                       off+j*cell_size + vec_matrix[i][j][1]*increment);
            ctx.stroke();
        }
    }
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