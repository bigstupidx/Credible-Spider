#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "Card.h"

class Poker {
    /*
    �����Ϲ�10���ƣ�ǰ4��6�ţ���6��5�ţ���6*4+5*6=54��
    ���乲5����ÿ��10�ţ���10*5=50��
    �ܼ�104�ţ�104=13*8��Ϊ������ȥ����С���õ�
    heart ����
    spade ����
    club ÷��
    diamond ����
    */
public:

    uint32_t seed;//����
    int suitNum;//��ɫ
    int score;//����
    int operation;//��������
    bool hasGUI;//�Ѽ���ͼƬ

    //��������
    std::vector<std::vector<Card>> desk;//0Ϊ������

    //������
    std::vector<std::vector<Card>> corner;//0Ϊ������

    //���������
    std::vector<std::vector<Card>> finished;

    Poker() : seed(-1), suitNum(-1), score(-1), operation(-1), hasGUI(false) {}

    //ͨ����� �����==8 �����Ƿ������
    bool isFinished();

    //���ص�ǰ���������ֵ
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