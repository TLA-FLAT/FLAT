/*
 * Copyright (C) 2015 The Language Archive - Max Planck Institute for Psycholinguistics
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package nl.mpi.tla.flat.deposit;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import org.slf4j.LoggerFactory;

/**
 * @author Menzo Windhouwer
 */
public class DoorKeeper {
    
    private static final org.slf4j.Logger logger = LoggerFactory.getLogger(DoorKeeper.class.getName());
    
    private static void showHelp() {
        System.err.println("INF: doorkeeper <OPTIONS> <workflow FILE> (<param>=<value>)*");
        System.err.println("INF: where <OPTIONS> are:");
        System.err.println("INF: -f <action> : from this action (name) (optional)");
        System.err.println("INF: -t <action> : to this action (name) (optional)");
    }
    
    static public void addParameter(Map<String,XdmValue> params,String name,String value) {
        XdmValue val = new XdmAtomicValue(value.replaceAll("\\{", "{{").replaceAll("\\}","}}"));
        if (params.containsKey(name))
            params.put(name,params.get(name).append(val));
        else
            params.put(name,val);
        logger.debug("parameter["+name+"]["+params.get(name)+"]");
    }

    public static void main(String[] args) {
        String start = null;
        String stop  = null;
        Map<String,XdmValue> params = new HashMap();
    
        OptionParser parser = new OptionParser("f:t:?*");
        OptionSet options = parser.parse(args);
        
        if (options.has("f"))
            start = (String)options.valueOf("f");
        if (options.has("t"))
            stop = (String)options.valueOf("t");
        
        if (options.has("?")) {
            showHelp();
            System.exit(0);
        }

        List arg = options.nonOptionArguments();
        if (arg.size()<1) {
            showHelp();
            System.exit(1);
        }

        String wfFile = (String)arg.get(0);
        File wf = new File(wfFile);
        if (!wf.isFile()) {
            logger.error("workflow["+wfFile+"] isn't a file!");
            showHelp();
            System.exit(1);
        }
        if (!wf.canRead()) {
            logger.error("workflow["+wfFile+"] isn't readable!");
            showHelp();
            System.exit(1);
        }
        
        for (int i=1;i<arg.size();i++) {
            String pv = (String)arg.get(i);
            if (pv.contains("=")) {
                addParameter(params,pv.split("=")[0], pv.split("=")[1]);
            } else {
                logger.error("workflow["+pv+"] isn't a valid <param>=<value>!");
                showHelp();
                System.exit(1);
            }
        }
        
        try {
            Flow flw = new Flow(wf,params);
            flw.run(start,stop);
        } catch (Exception ex) {
            logger.error("FATAL:",ex);
            System.exit(1);
        }
    }
}
