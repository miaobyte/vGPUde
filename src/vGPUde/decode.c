#include "hook.h"


DEFINE_HOOK_FUNC(CUresult, cuvidCreateVideoParser, 
    (CUvideoparser *pObj, CUVIDPARSERPARAMS *pParams), 
    (pObj, pParams))

// 拦截 cuvidCreateDecoder
DEFINE_HOOK_FUNC(CUresult, cuvidCreateDecoder, 
    (CUvideodecoder *pDecoder, CUVIDDECODECREATEINFO *pCreateInfo), 
    (pDecoder, pCreateInfo))

// 拦截 cuvidDecodePicture
DEFINE_HOOK_FUNC(CUresult, cuvidDecodePicture, 
    (CUvideodecoder hDecoder, CUVIDPICPARAMS *pPicParams), 
    (hDecoder, pPicParams))

// 拦截 cuvidParseVideoData
DEFINE_HOOK_FUNC(CUresult, cuvidParseVideoData, 
    (CUvideoparser hParser, CUVIDSOURCEDATAPACKET *pPacket), 
    (hParser, pPacket))