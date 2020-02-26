#include "glwidget.h"

#define ATTRIB_VERTEX 0
#define ATTRIB_TEXTURE 1
 
GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    textureY(QOpenGLTexture::Target2D),
    textureU(QOpenGLTexture::Target2D),
    textureV(QOpenGLTexture::Target2D),
    glBuffer(QOpenGLBuffer::VertexBuffer),
    yuvDataPtr(NULL),
    fileIndex(0)
{
    yuvDataPtr = new unsigned char[1280*720*3/2];
}
 
GLWidget::~GLWidget()
{

    delete pVShader;
    delete pFShader;
    delete [] yuvDataPtr;
    
    textureY.destroy();
    textureU.destroy();
    textureV.destroy();

    glBuffer.destroy();
}

void GLWidget::loadYUV()
{
    qDebug() << __FILE__ << ":" << __func__;
    
    pYUVFile = fopen("./frame--000000.bin", "rb");

    frameW = 1280;
    frameH = 720;

    int nLen = frameW*frameH*3/2;
    if(yuvDataPtr == NULL) {
        yuvDataPtr = new unsigned char[nLen];
    }
    fread(yuvDataPtr, 1, nLen, pYUVFile);

    fclose(pYUVFile);
}


void GLWidget::getYUVFrameName()
{
    
    std::stringstream ss;

    ss << "./frame-" << "-" << std::setw(6)
            << std::setfill('0') << fileIndex << ".bin";
    //std::cout << ss.str() << std::endl;
        filename = ss.str();

    ++fileIndex%=10;

}

void GLWidget::updateFrame()
{
    int nLen =0;
    nLen = frameW*frameH*3/2;

    if(fileIndex <= 301) {


    getYUVFrameName();

    pYUVFile = fopen(filename.c_str(), "rb");
    fread(yuvDataPtr, 1, nLen, pYUVFile);

    fclose(pYUVFile);

    frameW = 1280;
    frameH = 720;

    update();
    qDebug() << __FILE__ << ": " << __func__ << filename.c_str();
    }

    //qDebug() << __FILE__ << ": " << __func__ << filename.c_str();
}
 
void GLWidget::initializeGL()
{
    qDebug() << __FILE__ << ":" << __func__;
    bool bCompile;
    initializeOpenGLFunctions();
 
    glEnable(GL_DEPTH_TEST);

    static const GLfloat vertices[]{
        //Vertex
        -1.0f,-1.0f,
        -1.0f,+1.0f,
        +1.0f,+1.0f,
        +1.0f,-1.0f,
        //Texture
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
    };
 
    glBuffer.create();
    glBuffer.bind();
    glBuffer.allocate(vertices,sizeof(vertices));

    pVShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =  "attribute vec4 vertexIn; \
                        attribute vec2 textureIn; \
                        varying vec2 textureOut;  \
                        void main(void)           \
                        {                         \
                            gl_Position = vertexIn; \
                            textureOut = textureIn; \
                        }";

    bCompile = pVShader->compileSourceCode(vsrc);
    if(!bCompile)
    {
        qDebug() << __FILE__ << ":" << __func__<< ": Compile Vertex Shader fail";
    }

    pFShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =  "#ifdef GL_ES               \n"
                        "   precision mediump float;   \n"
                        "#endif \n"
                        "varying vec2 textureOut; \
                        uniform sampler2D tex_y; \
                        uniform sampler2D tex_u; \
                        uniform sampler2D tex_v; \
                        void main(void) \
                        { \
                        vec3 yuv; \
                        vec3 rgb; \
                        yuv.x = texture2D(tex_y, textureOut).r; \
                        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
                        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
                        rgb = mat3( 1,       1,         1, \
                                    0,       -0.39465,  2.03211, \
                                    1.13983, -0.58060,  0) * yuv; \
                        gl_FragColor = vec4(rgb, 1); \
                        }";

    bCompile = pFShader->compileSourceCode(fsrc);
    if(!bCompile)
    {
        qDebug() << __FILE__ << ":" << __func__<< ": Compile Fragment Shader fail";
    }

    shaderProgram.addShader(pVShader);
    shaderProgram.addShader(pFShader);

    shaderProgram.bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    shaderProgram.bindAttributeLocation("textureIn", ATTRIB_TEXTURE);

        // Link shader pipeline
    if (!shaderProgram.link())
        close();

    // Bind shader pipeline for use
    if (!shaderProgram.bind())
        close();

    shaderProgram.enableAttributeArray(ATTRIB_VERTEX);
    shaderProgram.enableAttributeArray(ATTRIB_TEXTURE);

    shaderProgram.setAttributeBuffer(ATTRIB_VERTEX,GL_FLOAT,0,2,2*sizeof(GLfloat));
    shaderProgram.setAttributeBuffer(ATTRIB_TEXTURE,GL_FLOAT,8*sizeof(GLfloat),2,2*sizeof(GLfloat));
 
    textureUniformY = shaderProgram.uniformLocation("tex_y");
    textureUniformU = shaderProgram.uniformLocation("tex_u");
    textureUniformV = shaderProgram.uniformLocation("tex_v");

    textureY.create();
    textureU.create();
    textureV.create();

    id_y = textureY.textureId();
    id_u = textureU.textureId();
    id_v = textureV.textureId();

    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glClearColor(0.0,0.0,0.0,0);

}
 
void GLWidget::paintGL()
{
    qDebug() << __FILE__ << ":" << __func__;
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#if 0
    GLfloat vertexPoints[]{
        -1.0, -1.0,0.0f, 1.0f,
            1.0, -1.0,1.0f, 1.0f,
            -1.0, 1.0,0.0f, 0.0f,
            1.0, 1.0,1.0f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), vertexPoints, GL_STATIC_DRAW);
#endif
    // activate texture 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_y);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frameW, frameH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, yuvDataPtr);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(textureUniformY, 0);

    // activate texture 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frameW/2, frameH/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (char*)yuvDataPtr+frameW*frameH);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(textureUniformU, 1);

    // activate texture 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frameW/2, frameH/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (char*)yuvDataPtr+frameW*frameH*5/4);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1i(textureUniformV, 2);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}
 
void GLWidget::resizeGL(int w, int h)
{
    qDebug() << __FILE__ << ":" << __func__ <<": FrameW x FrameH =" << w<< "x"<<h;
    glViewport(0,0,w,h);

    int err = glGetError();
    if (err) fprintf(stderr, "OpenGL Error 0x%x: %s.\n", err, __func__);
}
