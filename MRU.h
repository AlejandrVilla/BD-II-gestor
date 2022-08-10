#pragma once
#include "replacer.h"
#include <list>
#include <vector>

class MRU : public Replacer
{
public:
    explicit MRU(size_t num_pages);
    ~MRU();
    bool Victim (int32_t *frame_id) override;
    void Pin    (int32_t frame_id) override;
    void Unpin  (int32_t frame_id) override;
    size_t Size() override;
private:
    bool IsInReplacer(int32_t frame_id);
    std::list<int32_t> w_list{};
    std::vector<std::list<int32_t>::iterator> page_iters;
};

MRU::MRU(size_t num_pages){}

MRU::~MRU() = default;

bool MRU::Victim(int32_t *frame_id){
    if(w_list.empty()){
        *frame_id = -1;
        return false;
    }
    *frame_id = w_list.front();
    w_list.pop_front();
    page_iters[*frame_id] = std::list<int32_t>::iterator{};
    return true;
}

void MRU::Pin(int32_t frame_id){
    if(!IsInReplacer(frame_id)){
        return;
    }
    w_list.erase(page_iters[frame_id]);
    page_iters[frame_id] = std::list<int32_t>::iterator{};
}

void MRU::Unpin(int32_t frame_id) {
    if (IsInReplacer(frame_id)) {
        return;
    }
    w_list.push_front(frame_id);
    page_iters[frame_id] = w_list.begin();
}

size_t MRU::Size() {
    return w_list.size();
}

bool MRU::IsInReplacer(int32_t frame_id) {
  return page_iters[frame_id] != std::list<int32_t>::iterator{};
}