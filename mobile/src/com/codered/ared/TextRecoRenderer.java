package com.codered.ared;

import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

import com.qualcomm.QCAR.QCAR;

/** The renderer class for the TextReco sample. */
public class TextRecoRenderer implements GLSurfaceView.Renderer {
    public boolean mIsActive = false;

    /** Reference to main activity **/
    public TextReco mActivity;

    /** Native function for initializing the renderer. */
    public native void initRendering();

    /** Native function to update the renderer. */
    public native void updateRendering(int width, int height);

    private final List<String> tempList = new ArrayList<String>();
    private final List<String> mWords = new ArrayList<String>();

    /** Called when the surface is created or recreated. */
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        DebugLog.LOGD("GLRenderer::onSurfaceCreated");

        // Call native function to initialize rendering:
        initRendering();

        // Call QCAR function to (re)initialize rendering after first use
        // or after OpenGL ES context was lost (e.g. after onPause/onResume):
        QCAR.onSurfaceCreated();
    }

    /** Called when the surface changed size. */
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        DebugLog.LOGD("GLRenderer::onSurfaceChanged");

        // Call native function to update rendering when render surface
        // parameters have changed:
        updateRendering(width, height);

        // Call QCAR function to handle render surface size changes:
        QCAR.onSurfaceChanged(width, height);
    }

    /** The native render function. */
    public native void renderFrame();

    /** Called to draw the current frame. */
    public void onDrawFrame(GL10 gl) {
        if (!mIsActive) {
            mWords.clear();
            mActivity.updateWordListUI(mWords);
            return;
        }

        // Update render view (projection matrix and viewport) if needed:
        mActivity.updateRenderView();

        // Call our native function to render content
        renderFrame();

        List<String> words;
        synchronized (mWords) {
            words = new ArrayList<String>(mWords);
        }

        // update UI - we copy the list to avoid concurrent modifications
        mActivity.updateWordListUI(new ArrayList<String>(words));
    } 

    public void wordsStartLoop() {
        tempList.clear();
    }

    void addWord(String word) {
        tempList.add(word);
    }

    public void wordsEndLoop() {
        synchronized (mWords) {
            mWords.clear();
            mWords.addAll(tempList);
        }
    }
}
