#include "simple_decoder.h"
#include "ms_asr_utils.h"
#include <time.h>
#include <assert.h>
#include <unordered_map>
#include <vector> 
#include <limits>

#define DBG(format, ...) //printf(format, ##__VA_ARGS__)
#define DBG_LINE()  printf("###LINE%d\n", __LINE__)

#define DUMP_TOKS 0

extern int ms_asr_dbg_flag;

using namespace std;

typedef struct {
    float      beam_;
    uint8_t*  sfst_buf;
    uint32_t*  sym_buf;
    uint32_t   num_frames_decoded_;
}decoder_t;

//临时这么解决下。。
std::unordered_map<StateId, Token*> cur_toks_; //StateId 指的是当前tok的弧的结尾状态
std::unordered_map<StateId, Token*> prev_toks_;

static int max_toks = 0;

/**********************local function declare*********************************/
static void _ClearToks(unordered_map<StateId, Token*> &toks);

void _ProcessEmitting(decoder_t* decoder, producer_t* producer);
void _ProcessNonemitting(decoder_t* decoder);
void _PruneToks(unordered_map<StateId, Token*> *toks, float beam);

/**********************decoder private*********************************/
static uint32_t l_tmp_buf[ARC_LEN];
static uint32_t l_tmp_len;
void _remove_eps(uint32_t* buf, uint32_t* len)
{
    l_tmp_len = 0;
    for(uint32_t i=0; i<*len; i++) {
        if((buf[i] != 0) && (buf[i] != DUMMY_IDX)){
            l_tmp_buf[l_tmp_len] = buf[i];
            l_tmp_len++;
        }
    }
    memcpy(buf, l_tmp_buf, l_tmp_len*sizeof(uint32_t));
    *len = l_tmp_len;
    return;
}


static void dump_tok(decoder_t* decoder, Token* tok)
{
    std::vector<uint8_t*> arcs_reverse;  // arcs in reverse order.
    for (; tok != NULL; tok = tok->prev_) {
        arcs_reverse.push_back(tok->arc_);
    }
    //assert(arcs_reverse.back()->state_out == 0);
    arcs_reverse.pop_back();  // that was a "fake" token... gives no info.

    //反序pop出来就是正向的arc序列
    for (int32_t i = (arcs_reverse.size())-1; i >= 0; i--) {
        uint8_t* arc = arcs_reverse[i];
        uint32_t idx = sfst_get_arc_idx(decoder->sfst_buf, arc);
        printf("%d,", idx);
    }
    printf("\n");
    return;
}

//默认dump cur_toks_
static void dump_toks(decoder_t* decoder, const char* info)
{
    printf("Frame %d=================Dump toks %s:\n", decoder->num_frames_decoded_, info);
    int idx = 0;
    for (unordered_map<StateId, Token*>::iterator iter = cur_toks_.begin();
       iter != cur_toks_.end();
       ++iter) {
        printf("%3d: State %d, cost %.3f: ", idx, iter->first, iter->second->cost_);
        dump_tok(decoder, iter->second);
        idx += 1;
    }
    printf("\n");
    return;
}


static int stat_arc_cnt = 0;
static int stat_arcs_cnt = 0;
/**********************tok method*********************************/
static int tok_max = 0;
static int tok_cnt = 0;
Token* tok_new(sfst_arc_t* arc, float acoustic_cost, Token* prev)
{
    Token* tok = (Token*)malloc(sizeof(Token));
    if(tok == NULL) return NULL;
    
    tok->arc_  = arc->arc_addr;
    tok->prev_ = prev;
    tok->ref_count_ = 1;
    
    if (prev) {
        prev->ref_count_++;
        tok->cost_ = prev->cost_ + arc->cost + acoustic_cost;
    } else {
        tok->cost_ = arc->cost + acoustic_cost;
    }
    
    tok_cnt+=1; 
    if(tok_cnt>tok_max) {
        tok_max=tok_cnt;
        //printf("tok_max update to %d\n", tok_max);
    };
    return tok;
}

void tok_del(Token* tok)
{
    while (--tok->ref_count_ == 0) {
        Token *prev = tok->prev_;
        free(tok); tok_cnt-=1;
        if (prev == NULL) return;
        else tok = prev;
    }
    return;
}

static void _ClearToks(unordered_map<StateId, Token*> &toks)
{
    for (unordered_map<StateId, Token*>::iterator iter = toks.begin();
        iter != toks.end(); ++iter) {
        tok_del(iter->second);
    }
    toks.clear();
}


void _InitDecoding(decoder_t* decoder) 
{
    // clean up from last time:
    _ClearToks(cur_toks_);
    _ClearToks(prev_toks_);
    // initialize decoding:
    cur_toks_[0] = tok_new(&sfst_first_arc, 0.0, NULL);
    decoder->num_frames_decoded_ = 0;
    stat_arc_cnt = 0;
    stat_arcs_cnt = 0;
    _ProcessNonemitting(decoder);
    return;
}

void _AdvanceDecoding(decoder_t* decoder, producer_t* producer) 
{
    decoder->num_frames_decoded_ = 0;   //继续解码
    assert(decoder->num_frames_decoded_ >= 0);
    uint32_t num_frames_ready = producer->NumFramesReady(producer);

    assert(num_frames_ready >= decoder->num_frames_decoded_);
    uint32_t target_frames_decoded = num_frames_ready;
    clock_t start,finish;
    while (decoder->num_frames_decoded_ < target_frames_decoded) {
        // note: ProcessEmitting() increments decoder->num_frames_decoded_
        DBG("==========================================deal %d frame\n", decoder->num_frames_decoded_);
        start=clock();
        _ClearToks(prev_toks_);
        cur_toks_.swap(prev_toks_);
        _ProcessEmitting(decoder, producer);//dump_toks(decoder, "After _ProcessEmitting");
        _ProcessNonemitting(decoder);//dump_toks(decoder, "After _ProcessNonemitting");
        _PruneToks(&cur_toks_, decoder->beam_);//dump_toks(decoder, "After _PruneToks");
        finish=clock();
        //float Times=(float)(finish-start)/CLOCKS_PER_SEC;
        //printf("Frame %d use %.3f ms\n", decoder->num_frames_decoded_, Times*1000);
    }
}

void _ProcessEmitting(decoder_t* decoder, producer_t* producer) 
{
    uint32_t frame = decoder->num_frames_decoded_;
    // Processes emitting arcs for one frame.  Propagates from
    // prev_toks_ to cur_toks_.
    float cutoff = std::numeric_limits<float>::infinity();
    DBG("=========Start\n");
    sfst_iter_t sfst_iter;
    for (unordered_map<StateId, Token*>::iterator iter = prev_toks_.begin();
        iter != prev_toks_.end();
        ++iter) {
        StateId state = iter->first;
        Token *tok = iter->second;
        //assert(state == tok->arc_->state_out);
        DBG("====Start\n");
        DBG("state=%d\n", state);
        stat_arcs_cnt+=1;
        //iter all arcs in this state
        int res = sfst_iter_state_init(decoder->sfst_buf, state, &sfst_iter);
        if(res != 0) continue;

        sfst_arc_t arc;
        while(sfst_iter_state(&sfst_iter, &arc)) { 
            stat_arc_cnt+=1;
            if(arc.state_out == state) continue; //skip final arc
            if(arc.sym_in != 0) {      // propagate...
                float acoustic_cost = -producer->LogLikelihood(producer, frame, arc.sym_in);
                float total_cost = tok->cost_ + arc.cost + acoustic_cost;
                DBG("ilabel=%d, cost=%f\n", arc.sym_in, total_cost);

                if (total_cost >= cutoff) continue;
                if (total_cost + decoder->beam_  < cutoff) {
                    cutoff = total_cost + decoder->beam_;
                }
                Token *new_tok = tok_new(&arc, acoustic_cost, tok);
                unordered_map<StateId, Token*>::iterator find_iter
                    = cur_toks_.find(arc.state_out);
                if (find_iter == cur_toks_.end()) {
                    cur_toks_[arc.state_out] = new_tok;
                } else {
                    if ( find_iter->second->cost_ > new_tok->cost_ ) {
                        tok_del(find_iter->second);
                        find_iter->second = new_tok;
                    } else {
                        tok_del(new_tok);
                    }
                }
            }

        }
        DBG("====End\n");
    }
    DBG("=========End\n");
    decoder->num_frames_decoded_++;
    return;
}

void _ProcessNonemitting(decoder_t* decoder) 
{
    // Processes nonemitting arcs for one frame.  Propagates within
    // cur_toks_.
    std::vector<StateId> queue;
    float infinity = std::numeric_limits<float>::infinity();
    float best_cost = infinity;
    for (unordered_map<StateId, Token*>::iterator iter = cur_toks_.begin();
       iter != cur_toks_.end();
       ++iter) {
        queue.push_back(iter->first);
        best_cost = std::min(best_cost, iter->second->cost_);
    }
    float cutoff = best_cost + decoder->beam_;
    sfst_iter_t sfst_iter;

    while (!queue.empty()) {
        StateId state = queue.back();
        queue.pop_back();
        Token *tok = cur_toks_[state];
        assert(tok != NULL); // && state == tok->arc_->state_out);
        stat_arcs_cnt+=1;
        //iter all arcs in this state
        int res = sfst_iter_state_init(decoder->sfst_buf, state, &sfst_iter);
        if(res != 0) continue;

        sfst_arc_t arc;
        while(sfst_iter_state(&sfst_iter, &arc)) { 
            stat_arc_cnt+=1;
            if(arc.state_out == state) continue; //skip final arc
            if(arc.sym_in == 0) {      // propagate nonemitting only...
                const float acoustic_cost = 0.0;
                Token *new_tok = tok_new(&arc, acoustic_cost, tok);
                if (new_tok->cost_ > cutoff) {
                    tok_del(new_tok);
                } else {
                    unordered_map<StateId, Token*>::iterator find_iter
                        = cur_toks_.find(arc.state_out);
                    if (find_iter == cur_toks_.end()) {
                        cur_toks_[arc.state_out] = new_tok;
                        queue.push_back(arc.state_out);
                    } else {
                        if ( find_iter->second->cost_ > new_tok->cost_ ) {
                            tok_del(find_iter->second);
                            find_iter->second = new_tok;
                            queue.push_back(arc.state_out);
                        } else {
                            tok_del(new_tok);
                        }
                    }
                }
            }

        }
    }
    return;
}

void _PruneToks(unordered_map<StateId, Token*> *toks, float beam) 
{
    if (toks->empty()) {
        printf("No tokens to prune.\n");
        return;
    }
    float best_cost = std::numeric_limits<float>::infinity();
    int toks_num = 0;
    for (unordered_map<StateId, Token*>::iterator iter = toks->begin();
       iter != toks->end(); ++iter) {
        best_cost = std::min(best_cost, iter->second->cost_);
        toks_num +=1;
    }
    if(toks_num>max_toks) {
        max_toks = toks_num;
        //printf("max_toks update to %d\n", max_toks);
    }
    
    std::vector<StateId> retained;
    float cutoff = best_cost + beam;
    //printf("### best_cost=%f, cut_off=%f\n", best_cost, cutoff);
    for (unordered_map<StateId, Token*>::iterator iter = toks->begin();
       iter != toks->end(); ++iter) {
        if (iter->second->cost_ < cutoff)
            retained.push_back(iter->first);
        else
            tok_del(iter->second);
    }
    unordered_map<StateId, Token*> tmp;
    for (size_t i = 0; i < retained.size(); i++) {
        tmp[retained[i]] = (*toks)[retained[i]];
    }
    DBG("Pruned to %ld toks.\n", retained.size());
    tmp.swap(*toks);
    
    return;
}

void* _Init(float beam, uint8_t* sfst_buf, uint32_t* sym_buf)
{
    decoder_t* decoder= (decoder_t*)malloc(sizeof(decoder_t));
    if(decoder == NULL) return NULL;
    decoder->beam_    = beam;
    decoder->sfst_buf = sfst_buf;    
    decoder->sym_buf  = sym_buf;    
    decoder->num_frames_decoded_ = 0;    
    return (void*)decoder;
}

void _Deinit(decoder_t* decoder)
{
    _ClearToks(cur_toks_);
    _ClearToks(prev_toks_);
    free(decoder);
    return;
}


int _Decode(decoder_t* decoder, producer_t* producer)
{
    DBG_TIME_INIT();
    max_toks = 0;
    _InitDecoding(decoder);
    _AdvanceDecoding(decoder, producer);
    if(ms_asr_dbg_flag&DBGT_WFST)DBG_TIME("Decodeing");
    if(ms_asr_dbg_flag&DBG_LVCSR) {
        printf("    Frame CNT=%d, max_toks=%d\n", producer->NumFramesReady(producer), max_toks);
        printf("    read arcs cnt=%d; read arc cnt=%d\n", stat_arcs_cnt, stat_arc_cnt);
        printf("    tok_max alloc %d (%u KB)\n", tok_max, tok_max*sizeof(Token)/1024);
    }
    
    return (!cur_toks_.empty());
}



int _ReachedFinal(decoder_t* decoder) 
{
    for (unordered_map<StateId, Token*>::const_iterator iter = cur_toks_.begin();
        iter != cur_toks_.end();
        ++iter) {
        if (iter->second->cost_ != std::numeric_limits<float>::infinity() &&
            sfst_get_finalcost(decoder->sfst_buf, iter->first) != COST_INF)
          return 1;
    }
    return 0;
}

// Outputs an FST corresponding to the single best path
// through the lattice.
int _GetBestPath(decoder_t* decoder, decode_result_t* decode_result)
{
    Token *best_tok = NULL;
    int is_final = decoder_ReachedFinal(decoder); 
    if (!is_final) {
        for (unordered_map<StateId, Token*>::const_iterator iter = cur_toks_.begin();
             iter != cur_toks_.end();
             ++iter) {
            if (best_tok == NULL || best_tok->cost_ > iter->second->cost_ ) {
                best_tok = iter->second;
            }
        }
    } else {
        float infinity =std::numeric_limits<float>::infinity(),
            best_cost = infinity;
        for (unordered_map<StateId, Token*>::const_iterator iter = cur_toks_.begin();
             iter != cur_toks_.end();
             ++iter) {
            float this_cost = iter->second->cost_ + sfst_get_finalcost(decoder->sfst_buf, iter->first);
            if (this_cost != infinity && this_cost < best_cost) {
                best_cost = this_cost;
                best_tok = iter->second;
            }
        }
    }
    
    if (best_tok == NULL) return -1;  // No output.

    std::vector<uint8_t*> arcs_reverse;  // arcs in reverse order.
    for (Token *tok = best_tok; tok != NULL; tok = tok->prev_) {
        arcs_reverse.push_back(tok->arc_);
    }
    //assert(arcs_reverse.back()->state_out == 0);
    arcs_reverse.pop_back();  // that was a "fake" token... gives no info.

    //反序pop出来就是正向的arc序列
    decode_result->arcs_len = 0;
    for (int32_t i = (arcs_reverse.size())-1; i >= 0; i--) {
        uint8_t* arc_addr = arcs_reverse[i];
        decode_result->arcs[decode_result->arcs_len] = \
            sfst_get_arc_idx(decoder->sfst_buf, arc_addr);
        decode_result->arcs_len++;
    }

    //从states序列解码 输入，输出 symbol序列
    decode_result->sym_in_len = 0;
    decode_result->sym_out_len= 0;
    uint32_t arc_idx;
    for(uint32_t i=0; i<decode_result->arcs_len; i++) {
        arc_idx = decode_result->arcs[i];
        if(arc_idx == INIT_ARC_IDX) continue;
        sfst_arc_t arc; 
        int res = sfst_get_arc(decoder->sfst_buf, decoder->sym_buf, arc_idx, &arc); 
        if(res != 0) return -1; //解码出错
        decode_result->sym_in[decode_result->sym_in_len] = arc.sym_in;
        decode_result->sym_out[decode_result->sym_out_len] = arc.sym_out;
        decode_result->sym_in_len++;
        decode_result->sym_out_len++;
    }
    //去除sym_in序列里的<eps>
    _remove_eps(decode_result->sym_in, &decode_result->sym_in_len);
    _remove_eps(decode_result->sym_out, &decode_result->sym_out_len);
    //解码结束
    return 0;
}


/**********************extern for C*********************************/

extern "C"{
void* decoder_Init(float beam, uint8_t* sfst_buf, uint32_t* sym_buf)
{
    return _Init(beam, sfst_buf, sym_buf);
}

void decoder_Deinit(void* decoder)
{
    _Deinit((decoder_t*)decoder);
}

void decoder_PrintArcs(uint32_t* arcs, uint32_t arcs_len)
{
    printf("====DUMP RAW Data:\n");
    for(uint32_t i = 0; i < arcs_len; i++) {
        printf("%d,", arcs[i]);
    }
    printf("\n");
    return;
}

void decoder_PrintSymbols(uint32_t* syms, uint32_t syms_len, dict_t* dict)
{
    //Print raw data
    printf("====DUMP RAW Data:\n");
    for(uint32_t i = 0; i < syms_len; i++) {
        printf("%d,", syms[i]);
    }
    printf("\n");
    //Print symbol strings
    printf("====DUMP Symbol Strings:\n");
    for(uint32_t i = 0; i < syms_len; i++) {
        printf("%s ", dict_get(dict, syms[i]));
    }
    printf("\n");
    return;
}

int decoder_Decode(void* decoder, producer_t* producer)
{
    return _Decode((decoder_t*)decoder, producer);
}
int decoder_ReachedFinal(void* decoder)
{
    return _ReachedFinal((decoder_t*)decoder);
}
int decoder_GetBestPath(void* decoder, decode_result_t* decode_result)
{
    DBG_TIME_INIT();
    int res = _GetBestPath((decoder_t*)decoder, decode_result);
    //DBG_TIME("GetBestPath");
    return res;
}

int decoder_Decoding(void* decoder, producer_t* producer)
{
    DBG_TIME_INIT();
    _AdvanceDecoding((decoder_t* )decoder, producer);
    if(ms_asr_dbg_flag&DBGT_WFST)DBG_TIME("WFST");
    if(ms_asr_dbg_flag&DBG_LVCSR){
        printf("    Frame CNT=%d\n", producer->NumFramesReady(producer));
        printf("    max_toks=%d\n", max_toks);
        printf("    read arcs cnt=%d; read arc cnt=%d\n", stat_arcs_cnt, stat_arc_cnt);
        printf("    tok_max alloc %d (%u KB)\n", tok_max, tok_max*sizeof(Token)/1024);
    }
    
    return (cur_toks_.empty());
}

void decoder_Clear(void* decoder)
{
    _InitDecoding((decoder_t* )decoder);
    max_toks = 0;  
    return;    
}
    
}
