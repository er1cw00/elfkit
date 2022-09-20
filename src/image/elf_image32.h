#pragma once

#include <image/elf_image.h>

class elf_image32 : public elf_image {
public:
    elf_image32(elf_reader & reader);
    virtual ~elf_image32();

public:
    virtual bool load();
    virtual void unload();
protected:

};
