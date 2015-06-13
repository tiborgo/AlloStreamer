allofw = require "allofw"
allofwutils = require "allofwutils"

GL = allofw.GL3

w = new allofw.OpenGLWindow();
w.makeContextCurrent();
omni = new allofw.OmniStereo("allofw.yaml");
nav = new allofwutils.WindowNavigation(w, omni)

vertex_shader = "#version 330\n" + omni.getShaderCode() + """
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 color;
    out vec3 Color;
    void main() {
        Color = color;
        gl_Position = omni_render(omni_transform(position));
    }
"""

fragment_shader = "#version 330\n" + """
    in vec3 Color;
    layout(location = 0) out vec4 outputF;
    void main() {
        outputF = vec4(Color, 1.0);
    }
"""

composite_custom_shader = """
    uniform vec2 viewport_angles = vec2(50, 40);
    uniform float viewport_blur = 1;
    uniform vec3 viewport_y = vec3(0, 1, 0);
    uniform vec3 viewport_x = vec3(1, 0, 0);

    vec4 viewport_restrict(vec4 color) {
        float blur = 1.0 / (viewport_blur / 180.0 * PI);
        vec2 xyspan = viewport_angles / 180.0 * PI;
        vec2 xyangle = vec2(acos(dot(normalize(warp - warp * dot(warp, viewport_y)), viewport_x)),
                            acos(dot(normalize(warp - warp * dot(warp, viewport_x)), viewport_y)));
        vec2 xyscale = vec2(1, 1) - max(vec2(0, 0), min(vec2(1, 1), (xyspan / 2.0 - abs(xyangle - PI / 2.0)) * blur));
        return color * (1 - dot(xyscale, xyscale));
    }

    void main() {
        omni_composite_init();
        vec4 scene = omni_composite_scene();
        scene = viewport_restrict(scene);
        if((drawMask & kCompositeMask_Panorama) != 0) {
            vec4 panorama = omni_composite_panorama();
            scene = omni_blend_pm(scene, panorama);
        }
        omni_composite_final(scene);
    }
"""

# composite_shader_id = omni.compositeCustomizeShader(composite_custom_shader)

getShaderInfoLog = (shader) ->
    buffer = new Buffer(4)
    GL.getShaderiv(shader, GL.INFO_LOG_LENGTH, buffer)
    length = buffer.readUInt32LE(0)
    if length > 0
        buf = new Buffer(length)
        GL.getShaderInfoLog(shader, length, buffer, buf)
        buf.toString("utf-8")

getProgramInfoLog = (program) ->
    buffer = new Buffer(4)
    GL.getProgramiv(program, GL.INFO_LOG_LENGTH, buffer)
    length = buffer.readUInt32LE(0)
    if length > 0
        buf = new Buffer(length)
        GL.getProgramInfoLog(program, length, buffer, buf)
        buf.toString("utf-8")
    else
        null

compileShaders = () ->
    shader_v = GL.createShader(GL.VERTEX_SHADER)
    GL.shaderSource(shader_v, [vertex_shader])
    shader_f = GL.createShader(GL.FRAGMENT_SHADER)
    GL.shaderSource(shader_f, [fragment_shader])
    @program = GL.createProgram()

    GL.compileShader(shader_v)
    log = getShaderInfoLog(shader_v)
    if log?
        console.log(log)
    GL.compileShader(shader_f)
    log = getShaderInfoLog(shader_f)
    if log?
        console.log(log)

    GL.attachShader(program, shader_v)
    GL.attachShader(program, shader_f)

    GL.bindFragDataLocation(program, 0, "outputF")

    GL.linkProgram(program)
    log = getProgramInfoLog(program)
    if log?
        console.log(log)

setupBuffers = () ->
    @vertex_buffer = new GL.Buffer()
    @vertex_array = new GL.VertexArray()

    vertices = []

    cube = (x, y, z, size) ->
        c0 = 0.3
        c1 = 0.7
        v0 = [ x - size / 2, y - size / 2, z - size / 2, c0, c0, c0 ]
        v1 = [ x - size / 2, y - size / 2, z + size / 2, c0, c0, c1 ]
        v2 = [ x - size / 2, y + size / 2, z - size / 2, c0, c1, c0 ]
        v3 = [ x - size / 2, y + size / 2, z + size / 2, c0, c1, c1 ]
        v4 = [ x + size / 2, y - size / 2, z - size / 2, c1, c0, c0 ]
        v5 = [ x + size / 2, y - size / 2, z + size / 2, c1, c0, c1 ]
        v6 = [ x + size / 2, y + size / 2, z - size / 2, c1, c1, c0 ]
        v7 = [ x + size / 2, y + size / 2, z + size / 2, c1, c1, c1 ]

        insert = (x) ->
            vertices.push(x[0])
            vertices.push(x[1])
            vertices.push(x[2])
            vertices.push(x[3])
            vertices.push(x[4])
            vertices.push(x[5])

        insert(v0); insert(v1); insert(v3); insert(v0); insert(v3); insert(v2); # 0132;
        insert(v3); insert(v1); insert(v5); insert(v3); insert(v5); insert(v7); # 3157;
        insert(v3); insert(v7); insert(v6); insert(v3); insert(v6); insert(v2); # 3762;
        insert(v1); insert(v0); insert(v4); insert(v1); insert(v4); insert(v5); # 1045;
        insert(v5); insert(v4); insert(v6); insert(v5); insert(v6); insert(v7); # 5467;
        insert(v0); insert(v2); insert(v6); insert(v0); insert(v6); insert(v4); # 0264;

    for x in [-5..5]
        for y in [-5..5]
            for z in [-5..5]
                if x != 0 || y != 0 || z != 0
                    cube(x, y, z, 0.3)

    @total_vertices = vertices.length / 6

    GL.bindBuffer(GL.ARRAY_BUFFER, vertex_buffer)
    vertices_array = new Float32Array(vertices)
    GL.bufferData(GL.ARRAY_BUFFER, 4 * vertices_array.length, vertices_array, GL.STATIC_DRAW)

    GL.bindVertexArray(vertex_array)
    GL.enableVertexAttribArray(0)
    GL.enableVertexAttribArray(1)
    GL.bindBuffer(GL.ARRAY_BUFFER, vertex_buffer)
    GL.vertexAttribPointer(0, 3, GL.FLOAT, GL.FALSE, 24, 0)
    GL.vertexAttribPointer(1, 3, GL.FLOAT, GL.FALSE, 24, 12)

setupRender = () ->
    compileShaders()
    setupBuffers()

omni.onCaptureViewport () ->
    GL.clear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT)
    GL.useProgram(program.id())
    omni.setUniforms(program.id())
    GL.bindVertexArray(vertex_array)
    GL.drawArrays(GL.TRIANGLES, 0, total_vertices)


allounity = require("allounity_receiver");

video_source = new allounity.VideoSource("rtsp://127.0.0.1:60000/h264ESVideoTest");

cubemap_left = new GL.Texture()
cubemap_right = new GL.Texture()


render = () ->
    omni.capture()
    sz = w.getFramebufferSize()

    if video_source.nextFrame()
        GL.bindTexture(GL.TEXTURE_CUBE_MAP, cubemap_left)
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_MAG_FILTER, GL.LINEAR);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_MIN_FILTER, GL.LINEAR);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_S, GL.CLAMP_TO_EDGE);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_T, GL.CLAMP_TO_EDGE);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_R, GL.CLAMP_TO_EDGE);
        for i in [0..5]
            f = video_source.getCurrentFrame(i, 0)
            if f?
                GL.texImage2D(GL.TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL.RGB, f.width, f.height, 0, GL.RGB, GL.UNSIGNED_BYTE, f.pixels);
        GL.bindTexture(GL.TEXTURE_CUBE_MAP, cubemap_right)
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_MAG_FILTER, GL.LINEAR);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_MIN_FILTER, GL.LINEAR);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_S, GL.CLAMP_TO_EDGE);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_T, GL.CLAMP_TO_EDGE);
        GL.texParameteri(GL.TEXTURE_CUBE_MAP, GL.TEXTURE_WRAP_R, GL.CLAMP_TO_EDGE);
        for i in [0..5]
            f = video_source.getCurrentFrame(i, 1)
            if f?
                GL.texImage2D(GL.TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL.RGB, f.width, f.height, 0, GL.RGB, GL.UNSIGNED_BYTE, f.pixels);
        GL.bindTexture(GL.TEXTURE_CUBE_MAP, 0)
        console.log("Frame!")

    omni.composite(0, 0, sz[0], sz[1], {
        panorama: [ cubemap_left.id(), cubemap_right.id(), "cubemap" ]
    })
    w.swapBuffers();

setupRender()
render()

w.onRefresh(render)

timer = setInterval () ->
    nav.update()
    render()
    w.pollEvents()

w.onClose () -> clearInterval(timer)
