#pragma once

namespace cpdoccore { 
namespace odf_reader {

class office_element;

namespace text {
class paragraph;
}

class document_context
{
public:
    document_context();
    virtual ~document_context();

public:
    void set_last_paragraph(text::paragraph * Paragraph);
    text::paragraph * get_last_paragraph();
    
private:
    class Impl;
    Impl * impl_;
};

}
}
