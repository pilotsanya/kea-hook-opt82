The hook is fork of https://gist.github.com/heggi/90390c135bef31640bbb8f74e1cb0fb4

The hook changes option 61 from option 82 (ip-port/mac-port) and generates flex-id from option 82

IP match as 10.

If comment host4_identifier, then use only client-id in table hosts

You can use client-id and/or flex-id in table hosts

For example

    "hosts-database": {
         "type": "mysql",
         "name": "kea",
         "user": "kea",
         "password": "password",
         "host": "localhost",
         "port": 3306
    },
    "host-reservation-identifiers":  [ "client-id", "flex-id" ],
    
    "hooks-libraries": [
                {
                        "library": "/usr/lib64/kea/hooks/kea-hook-option82.so"
                }
                
    "loggers": [
        {
                "name": "kea-dhcp4.option82_hook",
                "output_options": [
                        {
                                "output": "/var/log/kea/kea-option82-hook.log"
                        }
                ],
                "severity": "INFO",
                "debuglevel": 99
        }

Insert to DB 4 - flex-id 3 - client-id

ip

    INSERT INTO hosts (dhcp_identifier, dhcp_identifier_type, dhcp4_subnet_id, ipv4_address) VALUES (concat('10.0.99.2-'),unhex('01')), 4, 1, INET_ATON('192.0.2.160'));

mac

    INSERT INTO hosts (dhcp_identifier, dhcp_identifier_type, dhcp4_subnet_id, ipv4_address) VALUES (unhex('1122334455662D01'), 4, 1, INET_ATON('192.0.2.160'));
