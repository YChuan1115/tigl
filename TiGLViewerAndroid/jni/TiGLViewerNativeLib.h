/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib */

#ifndef _Included_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
#define _Included_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_setAssetMgr
  (JNIEnv * env, jclass, jobject mgr);
/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    setJNICallbacks
 * Signature: (Lde/dlr/sc/tiglviewer/android/JNICallbacks;)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_setJNICallbacks
  (JNIEnv *, jclass, jobject);
/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    init
 * Signature: (IILde/dlr/sc/tiglviewer/android/HapticCallback;)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_init
  (JNIEnv *, jclass, jint, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    createScene
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_createScene
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    step
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_step
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    openFile
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_openFile
  (JNIEnv *, jclass, jstring);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    isFiletypeSupported
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_isFiletypeSupported
  (JNIEnv *, jclass, jstring);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    removeObjects
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_removeObjects
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    changeCamera
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_changeCamera
  (JNIEnv *, jclass, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    fitScreen
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_fitScreen
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    clearContents
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_clearContents
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    mouseButtonPressEvent
 * Signature: (FFII)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseButtonPressEvent
  (JNIEnv *, jclass, jfloat, jfloat, jint, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    mouseButtonReleaseEvent
 * Signature: (FFII)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseButtonReleaseEvent
  (JNIEnv *, jclass, jfloat, jfloat, jint, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    mouseMoveEvent
 * Signature: (FFI)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_mouseMoveEvent
  (JNIEnv *, jclass, jfloat, jfloat, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    keyboardDown
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_keyboardDown
  (JNIEnv *, jclass, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    keyboardUp
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_keyboardUp
  (JNIEnv *, jclass, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    setClearColor
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_setClearColor
  (JNIEnv *, jclass, jint, jint, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    getClearColor
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_getClearColor
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    loadObject
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_loadObject__Ljava_lang_String_2
  (JNIEnv *, jclass, jstring);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    loadObject
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_loadObject__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jstring, jstring);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    unLoadObject
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_unLoadObject
  (JNIEnv *, jclass, jint);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    getObjectNames
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_getObjectNames
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    tiglGetVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_tiglGetVersion
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    osgGetVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_osgGetVersion
  (JNIEnv *, jclass);

/*
 * Class:     de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib
 * Method:    occtGetVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_de_dlr_sc_tiglviewer_android_TiGLViewerNativeLib_occtGetVersion
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif