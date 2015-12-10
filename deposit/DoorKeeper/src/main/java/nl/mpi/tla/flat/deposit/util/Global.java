/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.mpi.tla.flat.deposit.util;

import java.text.SimpleDateFormat;
import java.util.LinkedHashMap;
import java.util.Map;
import static nl.mpi.tla.flat.deposit.SIP.CMD_NS;
import static nl.mpi.tla.flat.deposit.SIP.LAT_NS;

/**
 *
 * @author menzowi
 */
public class Global {
    final static public SimpleDateFormat ASOF = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss");
    final static public Map<String,String> NAMESPACES = new LinkedHashMap<>();
    
    static {
        NAMESPACES.put("cmd", CMD_NS);
        NAMESPACES.put("lat", LAT_NS);
    };
}
