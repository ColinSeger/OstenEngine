#include "message.h"

void add_message(MessageSystem& system, Message message){

}

void handle_message(MessageSystem& system){
    if(system.messages.size() > 0) return;
    Message message = system.messages.front();
    
    switch (message.type)
    {
    case MessageType::Load :
        // ModelLoader::load_model()
        break;
    
    default:
        break;
    }
}