import * as drag_n_drop from "./drag_n_drop.js";

// -- Const

const defaultReduceVal = 2;

const size_picker_no_select = "none"
const size_picker_custom = "custom"

const dict_size_picker = {
    "facebook_profile": 1.0,
    "facebook_cover": 820 / 312,
    "twitter_profile": 1.0,
    "twitter_cover": 3.0,
    "linkedin_profile": 1.0,
    "linkedin_cover": 4.0,
    "discord": 1.0,
}

// -- Functions

function roundToTwo(num) {
    return +(Math.round(num + "e+2") + "e-2");
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

// ---------------------- Let's roll ! ---------------------- //

// ---------------- Elements

// ------ Drag And Drop

const regionBeforeDrop = document.getElementById('region_before_drop');
const regionAfterDrop = document.getElementById('region_after_drop');

const dropRegion = document.getElementById('drop_region_playground');
const inputImageRegion = document.getElementById('input_image');

const exampleImages = document.getElementsByClassName('img_example')

let inputImage;


// --- Image Processing stuff

const size_picker = document.getElementById('size_picker');
const targetRatioElt = document.getElementById('target_ratio');

let countDeleteRows = 0;
let countDeleteCols = 0;

// - Canvas + Draw
const canvas_draw = document.getElementById('canvas_draw');
let ctx = null;

let output_width, output_height;

let pixelsBuffer = null;

// - C/C++ related stuff

let exports = null
let sharedMemory = null
initWebAssembler()

// - Other elements !

const x_minus_button = document.getElementById('x_minus_button');
const y_minus_button = document.getElementById('y_minus_button');

const process_button = document.getElementById('process_button');

const original_button = document.getElementById('original_button');
const gray_button = document.getElementById('gray_button');
const energy_button = document.getElementById('energy_button');
const color_col_button = document.getElementById('color_col_button');
const color_row_button = document.getElementById('color_row_button');

const download_button = document.getElementById('download_button');
const process_another_button = document.getElementById('process_another_button');


// ------ Drag And Drop

// Open file selector when clicked on the drop region
drag_n_drop.openFakeFileInput(dropRegion, handleFiles)

drag_n_drop.setDropCallbacks(dropRegion, handleDrop)

function handleDrop(e) {
    drag_n_drop.handleDrop(e, handleFiles)
}

function handleFiles(files) {
    for (let i = 0, len = files.length; i < len; i++) {
        if (drag_n_drop.validateImage(files[i])) {
            storeAndDisplayImage(files[i]);
            return;
        }
    }
}

function storeAndDisplayImage(image) {
    // previewing image
    inputImage = document.createElement("img");
    inputImageRegion.appendChild(inputImage);

    // read the image...
    let reader = new FileReader();
    reader.onload = function (e) {
        inputImage.src = e.target.result;
        waitForImageInputLoad().then(initResultImage)
    }

    reader.readAsDataURL(image);
}

// -- Examples images

for (let i = 0; i < exampleImages.length; i++) {
    exampleImages[i].addEventListener('click', handleSelectExampleImage);
}

function handleSelectExampleImage(e) {
    // previewing image
    inputImage = document.createElement("img");
    inputImageRegion.appendChild(inputImage);

    inputImage.src = e.target.src;
    waitForImageInputLoad().then(initResultImage)

}

async function waitForImageInputLoad() {
    while (inputImage == null || !inputImage.complete) {
        await sleep(10)
    }
}

function initResultImage() {
    output_width = inputImage.naturalWidth
    output_height = inputImage.naturalHeight

    canvas_draw.width = output_width;
    canvas_draw.height = output_height;

    ctx = canvas_draw.getContext('2d');

    // Draw image on canvas
    ctx.drawImage(inputImage, 0, 0)

    // Convert image to buffer !
    const imgData = ctx.getImageData(0, 0, output_width, output_height)
    pixelsBuffer = Uint8ClampedArray.from(imgData.data.slice())

    // Hide Drop region & make appear result regions
    regionBeforeDrop.style.display = 'none';
    regionAfterDrop.style.display = 'flex';
}

// --------- End of Drag & Drop

// ------ C / C++
function initWebAssembler() {
    fetch('static/js/main.wasm')
        .then(response => response.arrayBuffer())
        .then(bytes => WebAssembly.instantiate(bytes))
        .then(results => {
            // Exports will contains memory + functions
            exports = results.instance.exports;

            // Get the memory out of the exports.
            sharedMemory = results.instance.exports.memory;

        })
        // .then(_promise => testWebAssembler())
        .catch(console.error);
}

function testWebAssembler() {
    // Get the function out of the exports.

    let image_in = [
        12, 25, 10, 255, 10, 10, 10, 255, 10, 10, 10, 255,
        40, 35, 20, 255, 10, 90, 90, 255, 10, 10, 10, 255,
        56, 54, 96, 255, 45, 63, 10, 255, 10, 10, 10, 255,
        98, 77, 80, 255, 99, 99, 99, 255, 10, 10, 10, 255,
    ];
    const l = image_in.length
    const w = 3
    const h = 4

    let heap = new Uint8Array(sharedMemory.buffer, 0, 5 * l);
    console.log("Input Array")
    for (let i = 0; i < l; i++) {
        heap[i] = image_in[i];
    }
    let ind1 = 0
    for (let i = 0; i < h; i++) {
        let s = ""
        for (let j = 0; j < w; j++) {
            for (let k = 0; k < 4; k++) {
                s += heap[ind1]
                s += " "
                ind1++
            }
        }
        console.log(s)
    }

    // let result = instance.exports.convert_raw_buffer_to_gray_buffer(0, w, h);
    let result = exports.convert_raw_buffer_to_energy_buffer(0, w, h);
    console.log("\nOutput Array")
    let index = result
    for (let i = 0; i < h; i++) {
        let s = ""
        for (let j = 0; j < w; j++) {
            for (let k = 0; k < 4; k++) {
                s += heap[index]
                s += " "
                index++
            }
        }
        console.log(s)
    }

    console.log("Memory bytes : ", sharedMemory.buffer.byteLength)
    // mem_grow_to(5 * memory.buffer.byteLength)
    mem_grow_to(1000 * 1000 * 4 * 3)
    console.log("Memory bytes after grow : ", sharedMemory.buffer.byteLength)
}

function mem_grow_to(bytes) {
    const mem_size = sharedMemory.buffer.byteLength;
    if (mem_size < bytes) {
        sharedMemory.grow(Math.ceil((bytes - mem_size) / (64 * 1024)));
    }

}

// ---------- Image Processing

function write_pixels_buffer_in_shared_memory() {
    let mem = new Uint8Array(sharedMemory.buffer, 0, sharedMemory.buffer.byteLength);
    mem.set(pixelsBuffer);
}

function write_result_in_canvas(result) {
    const out_buffer = new Uint8ClampedArray(sharedMemory.buffer, result, 4 * output_width * output_height)

    applyUInt8ClampedBuffer(out_buffer, output_width, output_height)
}

function applyUInt8ClampedBuffer(buffer, img_width, img_height) {
    const imgData = new ImageData(buffer, img_width, img_height)
    ctx.putImageData(imgData, 0, 0, 0, 0, img_width, img_height)
}

// -- Set Callbacks
size_picker.addEventListener("change", sizePickerChange);

x_minus_button.addEventListener("click", reduceX);
y_minus_button.addEventListener("click", reduceY);

process_button.addEventListener("click", process);

original_button.addEventListener("click", changeImageToOriginal);
gray_button.addEventListener("click", changeImageToGray);
energy_button.addEventListener("click", changeImageToEnergy);
color_col_button.addEventListener("click", changeImageToColorCol);
color_row_button.addEventListener("click", changeImageToColorRow);

download_button.addEventListener("click", downloadImage);
process_another_button.addEventListener("click", resetPlayground);

// -- Callbacks definition

function sizePickerChange(_event) {
    const actual_ratio = output_width / output_height
    targetRatioElt.innerHTML = "<p>Actual ratio is <strong>" + roundToTwo(actual_ratio) + "</strong></p>"

    const value_read = size_picker.value;
    if (value_read === size_picker_custom) {
        console.log("Change Size picker to", value_read)
        x_minus_button.style.display = 'block';
        y_minus_button.style.display = 'block';
        return;
    }

    x_minus_button.style.display = "none";
    y_minus_button.style.display = "none";

    if (value_read === size_picker_no_select) {
        process_button.style.display = "none";
        return
    }

    process_button.style.display = "block";

    const target_ratio = dict_size_picker[value_read]
    targetRatioElt.innerHTML += "<p>Target ratio is <strong>" + roundToTwo(target_ratio) + "</strong></p>"

    countDeleteRows = 0
    countDeleteCols = 0

    if (actual_ratio < target_ratio) {
        countDeleteRows = Math.round(output_height * (1 - (actual_ratio / target_ratio)))
    } else if (actual_ratio > target_ratio) {
        countDeleteCols = Math.round(output_width * (1 - (target_ratio / actual_ratio)))
    }
    console.log("Should delete", countDeleteRows, "rows and", countDeleteCols, "cols")
}


function reduceX(_event) {
    deleteSomeCols(defaultReduceVal)
}

function reduceY(_event) {
    deleteSomeRows(defaultReduceVal)
}

function process(_event) {
    // Should delete cols
    deleteSomeCols(countDeleteCols);

    // Should delete rows
    deleteSomeRows(countDeleteRows);
}

function changeImageToOriginal(_event) {
    applyUInt8ClampedBuffer(pixelsBuffer, output_width, output_height)
}

function changeImageToGray(_event) {
    mem_grow_to(2 * pixelsBuffer.length)

    write_pixels_buffer_in_shared_memory();

    const result = exports.convert_raw_buffer_to_gray_buffer(0, output_width, output_height);

    write_result_in_canvas(result)
}

function changeImageToEnergy(_event) {
    mem_grow_to(3 * pixelsBuffer.length)

    write_pixels_buffer_in_shared_memory();

    const result = exports.convert_raw_buffer_to_energy_buffer(0, output_width, output_height);

    write_result_in_canvas(result)
}

function changeImageToColorCol(_event) {
    mem_grow_to(4 * pixelsBuffer.length)

    write_pixels_buffer_in_shared_memory();

    const result = exports.color_one_col_from_raw_buffer(0, output_width, output_height);

    write_result_in_canvas(result)
}

function changeImageToColorRow(_event) {
    mem_grow_to(8 * pixelsBuffer.length)

    write_pixels_buffer_in_shared_memory();

    const result = exports.color_one_row_from_raw_buffer(0, output_width, output_height);

    write_result_in_canvas(result)
}

function downloadImage(_event){
    const img_data = canvas_draw.toDataURL("image/jpeg");

    let a = document.createElement('a')
    a.href = img_data
    a.download = "perfect_image_size_res.jpg"
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
}

function resetPlayground(_event){
    // Hide Drop region & make appear result regions
    regionAfterDrop.style.display = 'none';
    regionBeforeDrop.style.display = 'flex';
    inputImageRegion.removeChild(inputImage);
}

// -- Methods

function deleteSomeCols(count) {
    if (count < 1) {
        return
    }
    mem_grow_to(8 * pixelsBuffer.length)

    write_pixels_buffer_in_shared_memory();
    console.log("Starting delete cols")
    const result = exports.delete_multi_col_from_raw_buffer(0, output_width, output_height, count);
    console.log("End of delete cols")

    reduceColsCount(count)

    write_result_in_canvas(result)
}

function deleteSomeRows(count) {
    if (count < 1) {
        return
    }
    mem_grow_to(8 * pixelsBuffer.length)

    write_pixels_buffer_in_shared_memory();

    console.log("Starting delete rows")
    const result = exports.delete_multi_row_from_raw_buffer(0, output_width, output_height, count);
    console.log("End of delete rows")

    reduceRowsCount(count)

    write_result_in_canvas(result)
}


function reduceColsCount(count) {
    if (count < 1) {
        return
    }
    canvas_draw.width -= count;
    output_width -= count;
}

function reduceRowsCount(count) {
    if (count < 1) {
        return
    }
    canvas_draw.height -= count;
    output_height -= count;
}
