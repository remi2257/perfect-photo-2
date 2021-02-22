const memory = new WebAssembly.Memory({
    initial: 256,
});

const imports = {
    env: {
        memoryBase: 0,
        memory: memory,
        tableBase: 0,
        table: new WebAssembly.Table({initial: 0, element: 'anyfunc'})
    }
};


fetch('../out/main.wasm').then(response =>
    response.arrayBuffer()
).then(bytes => WebAssembly.instantiate(bytes, imports)).then(results => {
    let instance = results.instance;

    const exports = instance.exports

    // Get the function out of the exports.
    const {bufferSum, memory} = results.instance.exports

    let image_in = [
        12, 25, 10, 255, 45, 63, 10, 255, 78, 64, 34, 255,
        40, 35, 20, 255, 10, 90, 90, 255, 78, 64, 34, 255,
        56, 54, 96, 255, 45, 63, 10, 255, 78, 64, 34, 255,
        98, 77, 80, 255, 45, 63, 10, 255, 78, 64, 34, 255,
    ];
    const l = image_in.length
    const w = 3
    const h = 4

    let heap = new Uint8Array(memory.buffer, 0, 5 * l);
    console.log("#####")
    for (let i = 0; i < l; i++) {
        heap[i] = image_in[i];
    }
    let ind1 = 0
    for (let i = 0; i < h; i++) {
        let s = ""
        for (let j = 0; j < w; j++) {
            for (let k = 0; k < 4; k++){
                s += heap[ind1]
                s += " "
                ind1++
            }
        }
        console.log(s)
    }

    // let result = instance.exports.convert_raw_buffer_to_gray_buffer(0, w, h);
    let result = instance.exports.convert_raw_buffer_to_energy_buffer(0, w, h);

    console.log("\n" + result + "\n")
    let index = result
    for (let i = 0; i < h; i++) {
        let s = ""
        for (let j = 0; j < w; j++) {
            for (let k = 0; k < 4; k++){
                s += heap[index]
                s += " "
                index++
            }
        }
        console.log(s)
    }
}).catch(console.error);
