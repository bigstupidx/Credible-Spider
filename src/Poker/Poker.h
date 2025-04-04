#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Card.h"

class Poker {
    /*
    桌面上共10摞牌，前4摞6张，后6摞5张，共6*4+5*6=54张
    角落共5叠，每叠10张，共10*5=50张
    总计104张，104=13*8，为两套牌去掉大小王得到
    heart 红桃
    spade 黑桃
    club 梅花
    diamond 方块
    */
public:

    uint32_t seed;//种子
    int suitNum;//花色
    int score;//分数
    int operation;//操作次数
    bool hasGUI;//已加载图片

    //桌上套牌
    std::vector<std::vector<Card>> desk;//0为最里面

    //发牌区
    std::vector<std::vector<Card>> corner;//0为最里面

    //已完成套牌
    std::vector<std::vector<Card>> finished;

    Poker() : seed(-1), suitNum(-1), score(-1), operation(-1), hasGUI(false) {}

    //通过检测 已完成==8 返回是否已完成
    bool isFinished();

    //返回当前局面的评估值
    int GetValue() const;

    void printCard(const std::vector<Card> &cards) const;
    void printCard(const std::vector<std::vector<Card>> &vvcards) const;
    void printCard(int deskIndex, int num) const;

    friend bool operator==(const Poker& lhs, const Poker& rhs);
    friend std::ostream& operator<<(std::ostream& out, const Poker& poker);
};

std::ostream& operator<<(std::ostream& out, const Poker& poker);
bool operator==(const Poker& lhs, const Poker& rhs);

namespace std {
template<> struct hash<Poker> {
    size_t operator()(const Poker& poker) const;
};
}