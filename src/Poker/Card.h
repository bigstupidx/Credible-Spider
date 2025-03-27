#pragma once

#include <string>
#include <iostream>
#include <memory>

#ifndef _CONSOLE
#include "TImage.h"
#endif

class Card {
private:
#ifndef _CONSOLE
    int z_index;
    bool visible;
    std::shared_ptr<TImage> img, imgBack;
#endif
public:

    int suit;//��ɫ 1 2 3 4

    int point;//���� 1-13

    bool show;//�Ƿ��ѷ���

    Card(int suit, int point, bool show) :
        suit(suit), point(point), show(show)
#ifndef _CONSOLE
        , visible(true), z_index(0), img(nullptr), imgBack(nullptr)
#endif
    {}
    Card(int suit, int point) : Card(suit, point, false) {}

    Card(const Card& card): suit(card.suit), point(card.point), show(card.show)
#ifndef _CONSOLE
        , z_index(card.z_index), visible(card.visible), img(card.img), imgBack(card.imgBack)
#endif
    {

    }

    void SetShow(bool show) {
        this->show = show;
    }

    //���ػ�ɫ C D H S
    char getSuit() const {
        switch (suit) {
        case 1:
            return 'C';//÷��
        case 2:
            return 'D';//����
        case 3:
            return 'H';//����
        case 4:
            return 'S';//����
        default:
            throw std::string("Error:'getSuit():' Undefined suit");
        }
    }

    //��ӡ��
    //eg. [S1]
    friend std::ostream& operator<<(std::ostream& out, const Card& card);

#ifndef _CONSOLE
    void SetVisible(bool visible) {
        this->visible = visible;
    }

    void SetZIndex(int z) {
        z_index = z;
    }

    int GetZIndex() const {
        return z_index;
    }

    void SetPos(POINT pt) {
        img->pt = pt;
        imgBack->pt = pt;
    }

    POINT GetPos() const {
        return img->pt;
    }

    void SetImage(std::shared_ptr<TImage> img, std::shared_ptr<TImage> imgBack) {
        this->img = img;
        this->imgBack = imgBack;
    }

    TImage& GetBackImage() {
        return *imgBack;
    }

    TImage& GetImage() {
        return *img;
    }

    void Draw(HDC hdc) {
        if (visible)
            if (show)
                img->Draw(hdc);
            else
                imgBack->Draw(hdc);
    }
#endif

};

bool operator==(const Card& lhs, const Card& rhs);

std::ostream& operator<<(std::ostream& out, const Card& card);