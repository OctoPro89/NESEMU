// Constants (match NES resolution)
const NES_WIDTH = 256;
const NES_HEIGHT = 240;
const canvas = document.getElementById("canvas");

let gl;
let shaderProgram;
let vao;
let texture;

let framebuffer = new Uint8Array(NES_WIDTH * NES_HEIGHT * 3); // RGB buffer

let keys = {
    z: false, x: false, c: false, f: false, u: false,
    shift: false, enter: false, space: false,
    up: false, down: false, left: false, right: false
};

function platform_init() {
    gl = canvas.getContext("webgl2");
    if (!gl) {
        alert("WebGL2 not supported");
        return;
    }

    canvas.width = NES_WIDTH;
    canvas.height = NES_HEIGHT;

    gl.viewport(0, 0, NES_WIDTH, NES_HEIGHT);
    gl.clearColor(0.1, 0.1, 0.1, 1.0);

    setupShaders();
    setupQuad();
    setupTexture();
    setupInput();
}

function compileShader(type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.error(gl.getShaderInfoLog(shader));
        throw new Error("Shader compilation failed");
    }
    return shader;
}

function setupShaders() {
    const vsSource = `#version 300 es
    in vec2 position;
    in vec2 texCoord;
    out vec2 TexCoord;
    void main() {
        TexCoord = texCoord;
        gl_Position = vec4(position, 0.0, 1.0);
    }`;

    const fsSource = `#version 300 es
    precision mediump float;
    in vec2 TexCoord;
    uniform sampler2D screenTexture;
    out vec4 FragColor;
    void main() {
        FragColor = texture(screenTexture, TexCoord);
    }`;

    const vs = compileShader(gl.VERTEX_SHADER, vsSource);
    const fs = compileShader(gl.FRAGMENT_SHADER, fsSource);

    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vs);
    gl.attachShader(shaderProgram, fs);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.error(gl.getProgramInfoLog(shaderProgram));
        throw new Error("Shader program link failed");
    }

    gl.useProgram(shaderProgram);
}

function setupQuad() {
    const quadVertices = new Float32Array([
        -1, 1, 0, 1,
        -1, -1, 0, 0,
        1, -1, 1, 0,
        -1, 1, 0, 1,
        1, -1, 1, 0,
        1, 1, 1, 1,
    ]);

    vao = gl.createVertexArray();
    gl.bindVertexArray(vao);

    const vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);

    const posAttrib = gl.getAttribLocation(shaderProgram, "position");
    const texAttrib = gl.getAttribLocation(shaderProgram, "texCoord");

    gl.enableVertexAttribArray(posAttrib);
    gl.vertexAttribPointer(posAttrib, 2, gl.FLOAT, false, 4 * 4, 0);

    gl.enableVertexAttribArray(texAttrib);
    gl.vertexAttribPointer(texAttrib, 2, gl.FLOAT, false, 4 * 4, 2 * 4);
}

function setupTexture() {
    texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);

    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, NES_WIDTH, NES_HEIGHT, 0, gl.RGB, gl.UNSIGNED_BYTE, framebuffer);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
}

function setupInput() {
    window.addEventListener("keydown", (e) => {
        mapKey(e.code, true);
    });

    window.addEventListener("keyup", (e) => {
        mapKey(e.code, false);
    });
}

function mapKey(code, down) {
    switch (code) {
        case "KeyZ": keys.z = down; break;
        case "KeyX": keys.x = down; break;
        case "KeyC": keys.c = down; break;
        case "KeyF": keys.f = down; break;
        case "KeyU": keys.u = down; break;
        case "Enter": keys.enter = down; break;
        case "ShiftLeft":
        case "ShiftRight": keys.shift = down; break;
        case "ArrowUp": keys.up = down; break;
        case "ArrowDown": keys.down = down; break;
        case "ArrowLeft": keys.left = down; break;
        case "ArrowRight": keys.right = down; break;
        case "Space": keys.space = down; break;
    }
}

function platform_render() {
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.bindVertexArray(vao);
    gl.useProgram(shaderProgram);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, NES_WIDTH, NES_HEIGHT, gl.RGB, gl.UNSIGNED_BYTE, framebuffer);

    gl.drawArrays(gl.TRIANGLES, 0, 6);
}

function platform_get_keys() {
    return keys;
}

function platform_put_pixel(x, y, r, g, b) {
    if (x < 0 || x >= NES_WIDTH || y < 0 || y >= NES_HEIGHT) return;
    const index = (y * NES_WIDTH + x) * 3;
    framebuffer[index] = r;
    framebuffer[index + 1] = g;
    framebuffer[index + 2] = b;
}

function platform_now() {
    return performance.now();
}

function platform_sleep_ms(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}