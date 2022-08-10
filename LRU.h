#pragma once
#include "replacer.h"
#include <list>
#include <vector>
class LRU : public Replacer
{
public:
    explicit LRU(size_t num_pages);
    ~LRU();
    bool Victim (int32_t *frame_id) override;
    void Pin    (int32_t frame_id) override;
    void Unpin  (int32_t frame_id) override;
    size_t Size() override;
private:
    bool IsInReplacer(int32_t frame_id);
    std::list<int32_t> w_list{};
    std::vector<std::list<int32_t>::iterator> page_iters;
};
    
LRU::LRU(size_t num_pages) : page_iters{num_pages} {}

LRU::~LRU() = default;

bool LRU::Victim(int32_t *frame_id){
    if(w_list.empty()){
        *frame_id = -1;
        return false;
    }
    *frame_id = w_list.back();
    w_list.pop_back();
    page_iters[*frame_id] = std::list<int32_t>::iterator{};
    return true;
}

void LRU::Pin(int32_t frame_id){
    if(!IsInReplacer(frame_id)){
        return;
    }
    w_list.erase(page_iters[frame_id]);
    page_iters[frame_id] = std::list<int32_t>::iterator{};
}

void LRU::Unpin(int32_t frame_id) {
    if (IsInReplacer(frame_id)) {
        return;
    }
    w_list.push_front(frame_id);
    page_iters[frame_id] = w_list.begin();
}

size_t LRU::Size() {
    return w_list.size();
}

bool LRU::IsInReplacer(int32_t frame_id) {
  return page_iters[frame_id] != std::list<int32_t>::iterator{};
}