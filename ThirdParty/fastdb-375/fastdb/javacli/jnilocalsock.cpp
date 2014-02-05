#include "sockio.h"

#define _JNI_IMPLEMENTATION_ 1

#include <jni.h>

jint JNICALL readLocalSocket(JNIEnv* env, jclass, jlong sock, jbyteArray array, jint offs, jint len)
{
    
    jbyte* buf = env->GetByteArrayElements(array, 0); 
    jint rc = ((socket_t*)sock)->read(buf + offs, 1, len);
    env->ReleaseByteArrayElements(array, buf, 0);

    buf = env->GetByteArrayElements(array, 0); 
    env->ReleaseByteArrayElements(array, buf, 0);
    return rc;
}    

void JNICALL writeLocalSocket(JNIEnv* env, jclass, jlong sock, jbyteArray array, jint offs, jint len)
{
    jbyte* buf = env->GetByteArrayElements(array, 0); 
    ((socket_t*)sock)->write(buf + offs, len);
    env->ReleaseByteArrayElements(array, buf, 0);
    
    buf = env->GetByteArrayElements(array, 0); 
    env->ReleaseByteArrayElements(array, buf, 0);
}    

void JNICALL closeLocalSocket(JNIEnv* env, jclass, jlong ptr)
{
    socket_t* sock = (socket_t*)ptr;
    sock->close();
    delete sock;    
}

jlong JNICALL openLocalSocket(JNIEnv* env, jclass, jstring adr, jint port)
{
    char const* address =  env->GetStringUTFChars(adr, 0);
    char* url = new char[strlen(address) + 8];
    sprintf(url, "%s:%d", address, port);
    env->ReleaseStringUTFChars(adr, address);
    socket_t* sock = socket_t::connect(url);
    delete[] url;
    return (jlong)sock;
}


jlong JNICALL createLocalSocket(JNIEnv* env, jclass, jint port)
{
    char buf[64];
    sprintf(buf, "localhost:%d", port);
    socket_t* sock = socket_t::create_local(buf, 0);
    return (jlong)sock;
}

jlong JNICALL acceptLocalSocket(JNIEnv* env, jclass, jlong sock)
{
    return (jlong)((socket_t*)sock)->accept();
}

void JNICALL cancelAcceptLocalSocket(JNIEnv* env, jclass, jlong sock)
{
    ((socket_t*)sock)->cancel_accept();
}

static JNINativeMethod localSocketMethods[] = { 
    {    
        "readSocket", 
        "(J[BII)I",
        &readLocalSocket
    }, 
    {    
        "writeSocket", 
        "(J[BII)V",
        &writeLocalSocket
    }, 
    { 
        "openSocket", 
        "(Ljava/lang/String;I)J",
        &openLocalSocket
    }, 
    { 
        "closeSocket", 
        "(J)V",
        &closeLocalSocket
    },
    {    
        "acceptSocket", 
        "(J)J",
        &acceptLocalSocket
    }, 
    {    
        "cancelAcceptSocket", 
        "(J)V",
        &cancelAcceptLocalSocket
    }, 
    { 
        "createSocket", 
        "(I)J",
        &createLocalSocket
    }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    JNIEnv *env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    // printf("JNI_OnLoad env=%p, class=%s\n", env, PACKAGE_NAME "/LocalSocket");
    jclass cls = env->FindClass(PACKAGE_NAME "/LocalSocket");
    if (cls == NULL) { 
        printf("Class %s not found\n", PACKAGE_NAME "/LocalSocket");
    } else { 
        env->RegisterNatives(cls, localSocketMethods, 7);    
    }
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *)
{
    JNIEnv *env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    jclass cls = env->FindClass(PACKAGE_NAME "/LocalSocket");
    env->UnregisterNatives(cls);
}






