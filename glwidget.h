#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include <unistd.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QFile>

#include <QOpenGLBuffer>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT


private Q_SLOTS:
    void    updateFrame();
 
public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    void        loadYUV();

    void        getYUVFrameName();

    //void        displayFrame(unsigned char f_ptr, int frame_W, frame_H);

protected:
    void        initializeGL() Q_DECL_OVERRIDE; 
    void        paintGL() Q_DECL_OVERRIDE;
    void        resizeGL(int w, int h) Q_DECL_OVERRIDE;

private:
    QOpenGLShader *pVShader;
    QOpenGLShader *pFShader;
    QOpenGLShaderProgram shaderProgram;

    GLuint textureUniformY;
    GLuint textureUniformU;
    GLuint textureUniformV;
    GLuint id_y;
    GLuint id_u;
    GLuint id_v;
    QOpenGLTexture textureY;
    QOpenGLTexture textureU;
    QOpenGLTexture textureV;

    int frameW;
    int frameH;
    unsigned char* yuvDataPtr;
    FILE* pYUVFile;
    int fileIndex;
    std::string filename;
    QOpenGLBuffer glBuffer;
};
#endif // GLWINDOW_H
