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
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
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
    
    private File base = null;
    
    private static final Logger logger = LoggerFactory.getLogger(Flow.class.getName());
    
    protected XdmNode spec = null;
    
    protected Context context = null;
    
    protected List<ActionInterface> noActions = new LinkedList<>();
    
    protected List<ActionInterface> initActions = noActions;
    
    protected List<ActionInterface> mainActions =  noActions;
    
    protected List<ActionInterface> exceptionActions = noActions;
    
    protected List<ActionInterface> finalActions = noActions;

    public Flow(File spec) throws DepositException {
        this(spec,new HashMap<String,XdmValue>());
    }
    
    public Flow(File spec,Map<String,XdmValue> params) throws DepositException {
        this(new StreamSource(spec),spec,params);
    }
    
    public Flow(Source spec) throws DepositException {
        this(spec,new HashMap<String,XdmValue>());
    }
    public Flow(Source spec,Map<String,XdmValue> params) throws DepositException {
        this(spec,null,params);
    }

    public Flow(Source spec,File base) throws DepositException {
        this(spec,base,new HashMap<String,XdmValue>());
    }

    public Flow(Source spec,File base,Map<String,XdmValue> params) throws DepositException {
        this.base = base;
        try {
            this.spec = Saxon.buildDocument(spec);
        } catch(SaxonApiException e) {
            throw new DepositException(e);
        }
        this.context = new Context(this.spec,params);
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
                // get parameters
                Map<String,XdmValue> params = new LinkedHashMap();
                context.loadParameters(params,Saxon.xpath(action, "parameter"),"parameter");
                // is there a classpath?
                String clazzPath = null;
                if (Saxon.hasAttribute(action,"classpath")) {
                    clazzPath = Saxon.avt(Saxon.xpath2string(action,"@classpath"),action,context.getProperties());
                }
                try {
                    Class<ActionInterface> face = null;
                    if (clazzPath != null) {
                        // expand the classpath
                        String[] paths = clazzPath.split(System.getProperty("path.separator"));
                        ArrayList<URL> urlPaths = new ArrayList<>();
                        int i=0;
                        for(String path:paths) {
                            List<String> pathList = new ArrayList<>();
                            if (path.endsWith("*.jar")) {
                                // asumes dir/*.jar
                                String dir = path.replaceAll("(.*)\\*\\.jar","$1");
                                if (base != null)
                                    dir = base.toURI().resolve(dir).getPath();
                                DirectoryStream<Path> stream = Files.newDirectoryStream(Paths.get(dir),"*.jar");
                                for (Path entry: stream)
                                    pathList.add(entry.toString());
                            } else
                                pathList.add(path);
                            for (String p:pathList) {
                                if (!(p.endsWith(".jar") || p.endsWith(System.getProperty("file.separator"))))
                                    p += System.getProperty("file.separator");
                                if (base != null) {
                                    urlPaths.add(base.toURI().resolve(p).toURL());
                                } else {
                                    // assumes we have absolute paths in the classpath
                                    if (p.startsWith(System.getProperty("file.separator")))
                                        p = "file:" + p;
                                    urlPaths.add(new URL(p));
                                }
                            }
                        }
                        URL a[] = new URL[urlPaths.size()];
                        a = urlPaths.toArray(a);
                        // use a classloader to load the action class (and the classes it uses) from the expanded classpath
                        URLClassLoader clazzLoader = new URLClassLoader(a) {
                            public Class loadClass(String name) throws ClassNotFoundException {
                                //Flow.logger.debug("load class["+name+"] from classpath"+Arrays.toString(getURLs()));
                                Class clazz = null;
                                try {
                                    // first try to find the class in the local classpath
                                    clazz = findClass(name);
                                } catch(ClassNotFoundException e) {
                                    // then try to find the class in the global classpath
                                    clazz = super.loadClass(name);
                                }
                                return clazz;
                            }
                        };
                        face = (Class<ActionInterface>) clazzLoader.loadClass(clazz);
                    } else {
                        // use the regular class loader to load the action class
                        face = (Class<ActionInterface>) Class.forName(clazz);
                    }
                    // instantiate the class and add it to the workflow
                    ActionInterface actionImpl = face.newInstance();
                    actionImpl.setName(name!=null?name:clazz);
                    actionImpl.setParameters(params);
                    flow.add(actionImpl);
                } catch (ClassNotFoundException | InstantiationException | IllegalAccessException | IOException e) {
                    Flow.logger.error(" couldn't load action["+name+"]["+clazz+"]! "+e.getMessage());
                    throw new DepositException(e);
                }
            } catch (SaxonApiException ex) {
                Flow.logger.error(" couldn't load actions! "+ex.getMessage());
                throw new DepositException(ex);
            }
        }
        return flow;
    }
    
    public Context getContext() {
        return this.context;
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