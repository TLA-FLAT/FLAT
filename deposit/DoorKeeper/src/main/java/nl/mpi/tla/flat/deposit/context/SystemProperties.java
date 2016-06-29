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

import java.util.Map;
import java.util.Properties;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class SystemProperties implements ImportPropertiesInterface {
    
    private static final Logger logger = (Logger) LoggerFactory.getLogger(CLIParameters.class.getName());
    
    @Override
    public void importProperties(String prefix,Map<String, XdmValue> props) {
        String pre = (prefix==null?"":prefix);
        Properties sprops = System.getProperties();
        for (Object name : sprops.keySet()) {
            props.put(pre+name.toString(),new XdmAtomicValue(sprops.get(name).toString()));
        }
    }
    
}
