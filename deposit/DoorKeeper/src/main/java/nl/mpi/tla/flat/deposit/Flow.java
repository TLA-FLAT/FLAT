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
package nl.mpi.tla.flat.deposit;

import java.io.File;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.action.ActionInterface;
import nl.mpi.tla.flat.deposit.util.Saxon;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class Flow {
    
    private static final Logger logger = LoggerFactory.getLogger(Flow.class.getName());
    
    protected XdmNode spec = null;
    
    protected Context context = null;
    
    protected List<ActionInterface> noActions = new LinkedList<>();
    
    protected List<ActionInterface> initActions = noActions;
    
    protected List<ActionInterface> mainActions =  noActions;
    
    protected List<ActionInterface> exceptionActions = noActions;
    
    protected List<ActionInterface> finalActions = noActions;

    public Flow(File spec) throws DepositException {
        this(new StreamSource(spec));
    }
    
    public Flow(Source spec) throws DepositException {
        try {
            this.spec = Saxon.buildDocument(spec);
        } catch(SaxonApiException e) {
            throw new DepositException(e);
        }
        this.context = new Context(this.spec);
        loadFlow();
    }
    
    private void loadFlow() throws DepositException {
        try {
            initActions = loadFlow(Saxon.xpath(spec, "/flow/init/action"));
            mainActions = loadFlow(Saxon.xpath(spec, "/flow/main/action"));
            exceptionActions = loadFlow(Saxon.xpath(spec, "/flow/exception/action"));
            finalActions = loadFlow(Saxon.xpath(spec, "/flow/config/final/action"));
        } catch (SaxonApiException ex) {
            throw new DepositException(ex);
        }
    }
    
    private List<ActionInterface> loadFlow(XdmValue actions) throws DepositException {
        List<ActionInterface> flow = new LinkedList<>();
        for (XdmItem action:actions) {
            try {
                String name = Saxon.xpath2string(action,"@name");
                String clazz = Saxon.xpath2string(action,"@class");
                if (Saxon.hasAttribute(action,"when")) {
                    if (!Saxon.xpath2boolean(action,Saxon.xpath2string(action,"@when"),context.getProperties())) {
                        continue;
                    }
                }
                Map<String,XdmValue> params = new LinkedHashMap();
                context.loadParameters(params,Saxon.xpath(action, "parameter"),"parameter");
                try {
                    Class<ActionInterface> face = (Class<ActionInterface>) Class.forName(clazz);
                    ActionInterface actionImpl = face.newInstance();
                    actionImpl.setName(name!=null?name:clazz);
                    actionImpl.setParameters(params);
                    flow.add(actionImpl);
                } catch (ClassNotFoundException | InstantiationException | IllegalAccessException e) {
                    Flow.logger.error(" couldn't load action["+name+"]["+clazz+"]! "+e.getMessage());
                    throw new DepositException(e);
                }
            } catch (SaxonApiException ex) {
                java.util.logging.Logger.getLogger(Flow.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        return flow;
    }
    
    public boolean run() throws DepositException {
        boolean next = initFlow();
        if (next) {
            try {
                next = mainFlow();
            } catch (Exception e) {
                exceptionFlow(e);
            } finally {
                finalFlow();
            }
        }
        return next;
    }
    
    public void setLogger(Logger logger) {
        context.setLogger(logger);
    }
        
    private boolean initFlow() throws DepositException {
        boolean next = true;
        for (ActionInterface action:initActions) {
            next = action.perform(context);
            if (!next)
                break;
        }
        return next;
    }

    private boolean mainFlow() throws DepositException {
        boolean next = true;
        for (ActionInterface action:mainActions) {
            next = action.perform(context);
            if (!next)
                break;
        }
        return next;
    }

    private boolean exceptionFlow(Exception e) throws DepositException {
        boolean next = true;
        if (exceptionActions.isEmpty())
            throw new DepositException(e);
        for (ActionInterface action:exceptionActions) {
            next = action.perform(context);
            if (!next)
                break;
        }
        return next;
    }

    private boolean finalFlow() throws DepositException {
        boolean next = true;
        for (ActionInterface action:finalActions) {
            next = action.perform(context);
            if (!next)
                break;
        }
        return next;
    }

}