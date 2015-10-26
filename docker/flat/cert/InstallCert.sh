#!/usr/bin/expect

spawn java -jar /tmp/InstallCert.jar localhost:8443 changeit

expect "Enter certificate to add to trusted keystore or 'q' to quit:"

send "1\r"

wait