#include <SimpleCM/Protocol>

#include "simplecm_export.h"

typedef Tp::SharedPtr<SimpleProtocol> SimpleProtocolPtr;

class SIMPLECM_EXPORT KDEConnectTelepathyProtocolFactory
{
public:
    static SimpleProtocolPtr simpleProtocol();
private:
    static Tp::WeakPtr<SimpleProtocol> s_protocol;
};
