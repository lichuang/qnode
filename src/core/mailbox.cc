/*
 * Copyright (C) codedump
 */

#include "core/mailbox.h"

Mailbox::Mailbox()
  active_(false) {
  pipe_.check_read();    
}

Mailbox::~Mailbox() {
}

