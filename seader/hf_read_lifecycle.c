#include "hf_read_lifecycle.h"

#ifdef MOMENTUM_UI_LANG_ZH_CN
#define SEADER_HF_TEXT(en, zh) (zh)
#else
#define SEADER_HF_TEXT(en, zh) (en)
#endif

SeaderHfCardSessionDecision
    seader_hf_read_on_card_detect(SeaderHfReadState state, bool sam_can_accept_card) {
    if(state != SeaderHfReadStateDetecting) {
        return SeaderHfCardSessionDecisionAbort;
    }

    if(!sam_can_accept_card) {
        return SeaderHfCardSessionDecisionAbort;
    }

    return SeaderHfCardSessionDecisionStartConversation;
}

bool seader_hf_read_is_waiting_for_progress(SeaderHfReadState state) {
    return state == SeaderHfReadStateConversationStarting ||
           state == SeaderHfReadStateConversationActive || state == SeaderHfReadStateFinishing;
}

bool seader_hf_read_should_timeout(
    SeaderHfReadState state,
    uint32_t elapsed_ms,
    uint32_t timeout_ms) {
    if(!seader_hf_read_is_waiting_for_progress(state)) {
        return false;
    }

    return elapsed_ms >= timeout_ms;
}

const char* seader_hf_read_failure_reason_text(SeaderHfReadFailureReason reason) {
    switch(reason) {
    case SeaderHfReadFailureReasonUnavailable:
        return SEADER_HF_TEXT("HF unavailable", "HF 不可用");
    case SeaderHfReadFailureReasonSamBusy:
        return SEADER_HF_TEXT("SAM not idle", "SAM 忙碌");
    case SeaderHfReadFailureReasonSamTimeout:
        return SEADER_HF_TEXT("SAM timeout", "SAM 超时");
    case SeaderHfReadFailureReasonBoardMissing:
        return SEADER_HF_TEXT("Reader lost", "读卡器断开");
    case SeaderHfReadFailureReasonProtocolError:
        return SEADER_HF_TEXT("Protocol error", "协议错误");
    case SeaderHfReadFailureReasonInternalState:
        return SEADER_HF_TEXT("Read state error", "读取状态错误");
    case SeaderHfReadFailureReasonNone:
    default:
        return SEADER_HF_TEXT("Read failed", "读取失败");
    }
}
