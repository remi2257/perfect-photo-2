function preventDefault(e) {
    e.preventDefault();
    e.stopPropagation();
}


export function openFakeFileInput(region, callback) {
    // open file selector when clicked on the drop region
    let fakeInput = document.createElement("input");
    fakeInput.type = "file";
    fakeInput.accept = "image/*";
    fakeInput.multiple = false;

    region.addEventListener('click', function () {
        fakeInput.click();
    });

    fakeInput.addEventListener("change", function () {
        callback(fakeInput.files);
    });

}

export function setDropCallbacks(region, callback) {
    region.addEventListener('dragenter', preventDefault, false);
    region.addEventListener('dragleave', preventDefault, false);
    region.addEventListener('dragover', preventDefault, false);
    region.addEventListener('drop', preventDefault, false);

    region.addEventListener('drop', callback, false);

}

export function validateImage(image) {
    // check the type
    let validTypes = ['image/jpeg', 'image/png', 'image/gif'];
    if (validTypes.indexOf(image.type) === -1) {
        // alert("Invalid File Type");
        return false;
    }

    // check the size
    let maxSizeInBytes = 10e6; // 10MB
    if (image.size > maxSizeInBytes) {
        alert("File too large");
        return false;
    }

    return true;
}

export function handleDrop(e, callback) {
    let dt = e.dataTransfer,
        files = dt.files;

    if (files.length) {
        callback(files);
    } else {

        // check for img
        let html = dt.getData('text/html'),
            match = html && /\bsrc="?([^"\s]+)"?\s*/.exec(html),
            url = match && match[1];

        if (url) {
            uploadImageFromURL(url);
        }

    }

    function uploadImageFromURL(url) {
        let img = new Image;
        let c = document.createElement("canvas");
        let ctx = c.getContext("2d");

        img.onload = function () {
            c.width = this.naturalWidth;     // update canvas size to match image
            c.height = this.naturalHeight;
            ctx.drawImage(this, 0, 0);       // draw in image
            c.toBlob(function (blob) {        // get content as PNG blob

                // call our main function
                callback([blob]);

            }, "image/png");
        };
        img.onerror = function () {
            alert("Error in uploading");
        }
        img.crossOrigin = "";              // if from different origin
        img.src = url;
    }
}
