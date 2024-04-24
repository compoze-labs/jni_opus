#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "opus_jni.h"
#include "opus.h"

static const char *JNIT_CLASS = "interop/PrintLine";

char logMsg[255];
OpusDecoder *dec;
OpusEncoder *enc;


opus_int32 SAMPLING_RATE;
int CHANNELS;
int FRAME_SIZE;

int APPLICATION_TYPE = OPUS_APPLICATION_VOIP;
const int MAX_PAYLOAD_BYTES = 1500;


static jboolean opus_init_encoder (JNIEnv *env, jobject obj, jint samplingRate, jint numberOfChannels, jint frameSize)
{
    FRAME_SIZE = frameSize;
    SAMPLING_RATE = samplingRate;
    CHANNELS = numberOfChannels;

    int error;
    int size;

    size = opus_encoder_get_size(1);
    enc = malloc(size);
    error = opus_encoder_init(enc, SAMPLING_RATE, CHANNELS, APPLICATION_TYPE);

    sprintf(logMsg, "Initialized Encoder with ErrorCode: %d", error);

    return error;
}

static jint opus_encode_bytes (JNIEnv *env, jobject obj, jshortArray in, jbyteArray out)
{
    jint inputArraySize = (*env)->GetArrayLength(env, in);
    jint outputArraySize = (*env)->GetArrayLength(env, out);

    jshort* audioSignal = (*env)->GetShortArrayElements(env, in, 0);

    unsigned char *data = (unsigned char*)calloc(MAX_PAYLOAD_BYTES,sizeof(unsigned char));
    int dataArraySize = opus_encode(enc, audioSignal, FRAME_SIZE, data, MAX_PAYLOAD_BYTES);

    if (dataArraySize >=0)
    {
        if (dataArraySize <= outputArraySize)
        {
            (*env)->SetByteArrayRegion(env,out,0,dataArraySize,data);
        }
        else
        {
            sprintf(logMsg, "Output array of size: %d to small for storing encoded data.", outputArraySize);
            return -1;
        }
    }

    (*env)->ReleaseShortArrayElements(env,in,audioSignal,JNI_ABORT);

    return dataArraySize;
}

static jboolean opus_release_encoder (JNIEnv *env, jobject obj)
{
    free(enc);

    return 1;
}

static jboolean opus_init_decoder (JNIEnv *env, jobject obj, jint samplingRate, jint numberOfChannels, jint frameSize){
    FRAME_SIZE = frameSize;
    SAMPLING_RATE = samplingRate;
    CHANNELS = numberOfChannels;

    int size;
    int error;

    size = opus_decoder_get_size(CHANNELS);
    dec = malloc(size);
    error = opus_decoder_init(dec, SAMPLING_RATE, CHANNELS);

    return error;
}

static jint opus_decode_bytes (JNIEnv *env, jobject obj, jbyteArray in, jshortArray out){
    jint inputArraySize = (*env)->GetArrayLength(env, in);
    jint outputArraySize = (*env)->GetArrayLength(env, out);

    jbyte* encodedData = (*env)->GetByteArrayElements(env, in, 0);
    opus_int16 *data = (opus_int16*)calloc(outputArraySize,sizeof(opus_int16));

    int decodedDataArraySize = opus_decode(dec, encodedData, inputArraySize, data, FRAME_SIZE, 0);

    if (decodedDataArraySize >=0)
    {
        if (decodedDataArraySize <= outputArraySize)
        {
            (*env)->SetShortArrayRegion(env,out,0,decodedDataArraySize,data);
        }
        else
        {
            return -1;
        }
    }

    (*env)->ReleaseByteArrayElements(env,in,encodedData,JNI_ABORT);

    return decodedDataArraySize;
}

static jboolean opus_release_decoder (JNIEnv *env, jobject obj){
    free(dec);
    return 1;
}


static JNINativeMethod funcs[] = {
        { "opus_init_decoder", "(III)Z", (void *)&opus_init_encoder },
        { "opus_decode_bytes", "([B[S)I", (void *)&opus_encode_bytes },
        { "opus_release_decoder", "()Z", (void *)&opus_release_encoder },
        { "opus_init_decoder", "(III)Z", (void *)&opus_init_decoder },
        { "opus_decode_bytes", "([B[S)I", (void *)&opus_decode_bytes },
        { "opus_release_decoder", "()Z", (void *)&opus_release_decoder }
};

#define CURRENT_JNI JNI_VERSION_1_6

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    jclass  cls;
    jint    res;

    (void)reserved;

    if ((*vm)->GetEnv(vm, (void **)&env, CURRENT_JNI) != JNI_OK)
        return -1;

    cls = (*env)->FindClass(env, JNIT_CLASS);
    if (cls == NULL)
        return -1;

    res = (*env)->RegisterNatives(env, cls, funcs, sizeof(funcs)/sizeof(*funcs));
    if (res != 0)
        return -1;

    return CURRENT_JNI;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
JNIEnv *env;
jclass  cls;

(void)reserved;

if ((*vm)->GetEnv(vm, (void **)&env, CURRENT_JNI) != JNI_OK)
return;

cls = (*env)->FindClass(env, JNIT_CLASS);
if (cls == NULL)
return;

(*env)->UnregisterNatives(env, cls);
}
