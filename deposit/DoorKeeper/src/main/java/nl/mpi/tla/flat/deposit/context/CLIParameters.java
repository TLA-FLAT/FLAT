/*
 * Copyright (C) 2015 menzowi
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
package nl.mpi.tla.flat.deposit.context;

import java.util.HashMap;
import java.util.Map;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class CLIParameters implements ImportPropertiesInterface {
    
    private static final Logger logger = (Logger) LoggerFactory.getLogger(CLIParameters.class.getName());
    
    static final Map<String,XdmValue> params = new HashMap();
    
    static public void addParameter(String name,String value) {
        XdmValue val = new XdmAtomicValue(value);
        if (params.containsKey(name))
            params.put(name,params.get(name).append(val));
        else
            params.put(name,val);
        logger.debug("parameter["+name+"]["+params.get(name)+"]");
    }

    @Override
    public void importProperties(String prefix,Map<String, XdmValue> props) {
        String pre = (prefix==null?"":prefix);
        for (String name:params.keySet()) {
            if (props.containsKey(pre+name))
                props.put(pre+name,props.get(pre+name).append(params.get(name)));
            else
                props.put(pre+name,params.get(name));
            logger.debug("property["+pre+name+"]["+props.get(pre+name)+"]");
        }
    }
    
}
