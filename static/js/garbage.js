
// ************************************************************** //

// -------------------------- Garbage -------------------------- //

// ************************************************************** //

function createBufferFromGray() {
    pixelsBuffer = []

    for (let i = 0; i < output_height; i++) {
        for (let j = 0; j < output_width; j++) {
            for (let k = 0; k < 4; k++) {
                if (k === 3) {
                    pixelsBuffer.push(255)
                } else {
                    pixelsBuffer.push(grayMat[i][j])
                }
            }
        }
    }
}


function createBufferFromEnergyMatrix() {
    pixelsBuffer = []

    for (let i = 0; i < output_height; i++) {
        for (let j = 0; j < output_width; j++) {
            let val = energyMat[i][j];
            for (let k = 0; k < 3; k++) {
                pixelsBuffer.push(val)
            }
            pixelsBuffer.push(255)
        }
    }
}

function measureReduceX(event) {

    let start = new Date().getTime();

    // extractRGBFromBuffer(); // Remplir les matrices R, G et B
    extractGrayMatFromBuffer(); // Remplir la Mat Gray
    // extractGrayBufferFromBuffer(); // Remplir la Mat Gray
    let t_buff_to_gray = new Date().getTime();
    console.log('Buffer ➔ Gray Mat :', t_buff_to_gray - start, 'ms');

    computeEnergyMatrix()
    // computeEnergyBuffer()
    let t_gray_to_energy = new Date().getTime();
    console.log('Gray Mat ➔ Energy Mat :', t_gray_to_energy - t_buff_to_gray, 'ms');

    // -- Compute MinEnergy & Get Best Path

    computeMinEnergyMatReduceX()
    let t_energy_mat_to_min_energy_mat = new Date().getTime();
    console.log('Energy Mat ➔ Min Energy Mat :', t_energy_mat_to_min_energy_mat - t_gray_to_energy, 'ms');

    const best_path = findBestPathReduceX()
    let t_min_energy_mat_to_best_path = new Date().getTime();
    console.log('Min Energy Mat ➔ Best Path :', t_min_energy_mat_to_best_path - t_energy_mat_to_min_energy_mat, 'ms');

    // -- Create new image and apply it to canvas

    updateEverythingWithPathCols(best_path);
    // createBufferFromGray();
    let t_res_to_buffer = new Date().getTime();
    console.log('Best Path + Old Img (Im Res) ➔ Buffer :', t_res_to_buffer - t_min_energy_mat_to_best_path, 'ms');

    reduceColsCount()
    applyChanges(); // Applique la nouvelle image
    // applyChanges();
    let t_buffer_to_canvas = new Date().getTime();
    console.log('Buffer ➔ Canvas :', t_buffer_to_canvas - t_res_to_buffer, 'ms');

    // -- Update Min Energy Mat according to changes

    updateMinEnergyMat(best_path)
    let t_best_path_deletion = new Date().getTime();
    console.log('Best Path Deletion ➔ Im Res :', t_best_path_deletion - t_buffer_to_canvas, 'ms');

}

function computeEnergyBuffer() {
    energyBuffer = []
    let tmp;
    let index = 0;
    for (let i = 0; i < output_height - 1; i++) {
        for (let j = 0; j < output_width - 1; j++) {
            tmp = grayBuffer[index]
            energyBuffer.push(
                Math.abs(tmp - grayBuffer[index + 1]) +
                Math.abs(tmp - grayBuffer[index + output_height])
            )
            index++;
        }
        energyBuffer.push(0) // Finish row
    }
    for (let j = 0; j < output_width - 1; j++) { // Add last row
        energyBuffer.push(0)
    }
}

function extractGrayBufferFromBuffer() {
    let index = 0;
    grayBuffer = []
    for (let i = 0; i < output_height; i++) {
        for (let j = 0; j < output_width; j++) {
            grayBuffer.push(0.3 * pixelsBuffer[index] + 0.59 * pixelsBuffer[index + 1] + 0.11 * pixelsBuffer[index + 2])
            index += 4;
        }
    }
}
