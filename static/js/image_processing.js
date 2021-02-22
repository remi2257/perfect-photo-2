function hueToRGB(h) {
    // Must be fractions of 1
    const s = 1.0;
    const l = 0.5;

    let c = (1 - Math.abs(2 * l - 1)) * s,
        x = c * (1 - Math.abs((h / 60) % 2 - 1)),
        m = l - c / 2,
        r = 0,
        g = 0,
        b = 0;
    if (0 <= h && h < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (60 <= h && h < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (120 <= h && h < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (180 <= h && h < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (240 <= h && h < 300) {
        r = x;
        g = 0;
        b = c;
    } else if (300 <= h && h < 360) {
        r = c;
        g = 0;
        b = x;
    }
    r = Math.round((r + m) * 255);
    g = Math.round((g + m) * 255);
    b = Math.round((b + m) * 255);

    return [r, g, b];

}

export function transpose(original) {
    let copy = [];
    for (let j = 0; j < original[0].length; ++j) { // Create all rows
        copy.push([]);
    }
    for (let i = 0; i < original.length; ++i) {
        for (let j = 0; j < original[i].length; ++j) {
            copy[j].push(original[i][j]);
        }
    }
    return copy;
}

export function createBufferFromArray2D(array, normalize = false) {
    let normVal = 1;
    if (normalize) {
        let maxRow = array.map(function (row) {
            return Math.max.apply(Math, row);
        });
        let max = Math.max.apply(null, maxRow);

        normVal = max / 255;

        // console.log("Max read :", Math.round(max), "Norm val :", roundToTwo(normVal))
    }

    let buffer = []
    let r_g_b;

    const h = array.length;
    const w = array[0].length;

    for (let i = 0; i < h; i++) {
        for (let j = 0; j < w; j++) {
            let val = array[i][j];
            if (normalize) {
                val /= normVal;
                // for (let k = 0; k < 3; k++) {
                //     buffer.push(val)
                // }
                r_g_b = hueToRGB(255 - val)
                buffer.push(r_g_b[0])
                buffer.push(r_g_b[1])
                buffer.push(r_g_b[2])

            } else {
                for (let k = 0; k < 3; k++) {
                    buffer.push(val)
                }
            }
            buffer.push(255)
        }
    }
    return buffer
}

    export function flipBuffer(buffer, currentW, currentH) {
        let newBuffer = [];
        const offsetRow = 4 * currentW
        for (let i = 0; i < currentW; i++) {
            let index = 4 * i;
            for (let j = 0; j < currentH; j++) {
                newBuffer.push(buffer[index])
                newBuffer.push(buffer[index + 1])
                newBuffer.push(buffer[index + 2])
                newBuffer.push(buffer[index + 3])
                index += offsetRow
            }
        }
        return newBuffer;
}
