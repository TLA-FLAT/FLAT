package nl.mpi.tla.lat2fox;

import com.twmacinta.util.MD5;
import java.io.File;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;
import net.sf.saxon.trans.XPathException;
import net.sf.saxon.value.SequenceType;
import net.sf.saxon.value.StringValue;
import net.sf.saxon.om.StructuredQName; 
import net.sf.saxon.om.Sequence;
import net.sf.saxon.expr.XPathContext;
import net.sf.saxon.lib.ExtensionFunctionCall;
import net.sf.saxon.lib.ExtensionFunctionDefinition;
import net.sf.saxon.Configuration;
import net.sf.saxon.om.AxisInfo;
import net.sf.saxon.om.NodeInfo;
import net.sf.saxon.s9api.Processor;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XPathCompiler;
import net.sf.saxon.s9api.XPathExecutable;
import net.sf.saxon.s9api.XPathSelector;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.tree.NamespaceNode;
import net.sf.saxon.tree.iter.AxisIterator;

public final class SaxonExtensionFunctions {
    /**
     * Registers with Saxon 9.2+ all the extension functions 
     * <p>This method must be invoked once per TransformerFactory.
     *
     * @param factory the TransformerFactory pointing to
     * Saxon 9.2+ extension function registry 
     * (that is, a <tt>net.sf.saxon.Configuration</tt>). 
     * This object must be an instance of 
     * <tt>net.sf.saxon.TransformerFactoryImpl</tt>.
     */
    public static void registerAll(Configuration config) 
        throws Exception {
        config.registerExtensionFunction(new FileExistsDefinition());
        config.registerExtensionFunction(new CheckURLDefinition());
        config.registerExtensionFunction(new EvaluateDefinition());
        config.registerExtensionFunction(new MD5Definition());
        config.registerExtensionFunction(new FileSizeDefinition());
    }

    // -----------------------------------------------------------------------
    // sx:fileExists
    // -----------------------------------------------------------------------

    public static final class FileExistsDefinition 
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx", 
                                       "java:nl.mpi.tla.saxon", 
                                       "fileExists");
        }

        public int getMinimumNumberOfArguments() {
            return 1;
        }

        public int getMaximumNumberOfArguments() {
            return 1;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_ANY_URI };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.SINGLE_BOOLEAN;
        }
        
        public boolean dependsOnFocus() {
           return false;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {
                        URI uri = new URI(((StringValue) arguments[0].head()).getStringValue());
                        boolean exists = (new java.io.File(uri)).exists();
                        seq = (new XdmAtomicValue(exists)).getUnderlyingValue();
                    } catch(Exception e) {
                        System.err.println("ERR: ["+((StringValue) arguments[0].head()).getStringValue()+"]:"+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }

    // -----------------------------------------------------------------------
    // sx:checkURL
    // -----------------------------------------------------------------------

    public static final class CheckURLDefinition 
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx", 
                                       "java:nl.mpi.tla.saxon", 
                                       "checkURL");
        }

        public int getMinimumNumberOfArguments() {
            return 1;
        }

        public int getMaximumNumberOfArguments() {
            return 1;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_STRING };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.SINGLE_BOOLEAN;
        }
        
        public boolean dependsOnFocus() {
           return false;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {
                        String url = ((StringValue) arguments[0].head()).getStringValue();
                        boolean valid = true;
                        try {
                            URL u = new URL(url);
                        } catch(MalformedURLException e) {
                            valid = false;
                            System.err.println("WRN: URL["+url+"] is invalid["+e+"]!");
                        }
                        //System.err.println("DBG: URL["+url+"] valid?["+valid+"]");
                        seq = (new XdmAtomicValue(valid)).getUnderlyingValue();
                    } catch(Exception e) {
                        System.err.println("ERR: "+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }
    
    // -----------------------------------------------------------------------
    // sx:evaluate
    // -----------------------------------------------------------------------

    public static final class EvaluateDefinition
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx",
                                       "java:nl.mpi.tla.saxon",
                                       "evaluate");
        }

        public int getMinimumNumberOfArguments() {
            return 2;
        }

        public int getMaximumNumberOfArguments() {
            return 3;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_NODE, SequenceType.SINGLE_STRING, SequenceType.OPTIONAL_NODE };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.ANY_SEQUENCE;
        }
        
        public boolean dependsOnFocus() {
           return true;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {                
                        NodeInfo    node = (NodeInfo) arguments[0].head();
                        StringValue path = (StringValue) arguments[1].head();
                        NodeInfo    ns   = node;
                        if (arguments.length==3)
                            ns = (NodeInfo) arguments[2].head();
                        Processor processor = new Processor(context.getConfiguration());
                        XPathCompiler xpc   = processor.newXPathCompiler();
                        AxisIterator iter = ns.iterateAxis(AxisInfo.NAMESPACE);
                        NamespaceNode n = (NamespaceNode)iter.next();
                        while (n!=null) {
                            xpc.declareNamespace(n.getLocalPart(),n.getStringValue());
                            n = (NamespaceNode)iter.next();
                        }
                        XPathExecutable xpe = xpc.compile(path.getStringValue());
                        XPathSelector xps   = xpe.load();
                        xps.setContextItem(new XdmNode(node));
                        seq = xps.evaluate().getUnderlyingValue();
                    } catch(SaxonApiException e) {
                        System.err.println("ERR: "+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }
    
    // -----------------------------------------------------------------------
    // sx:md5
    // -----------------------------------------------------------------------

    public static final class MD5Definition 
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx", 
                                       "java:nl.mpi.tla.saxon", 
                                       "md5");
        }

        public int getMinimumNumberOfArguments() {
            return 1;
        }

        public int getMaximumNumberOfArguments() {
            return 1;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_ANY_URI };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.SINGLE_STRING;
        }
        
        public boolean dependsOnFocus() {
           return false;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {
                        URI uri = new URI(((StringValue) arguments[0].head()).getStringValue());
                        String hash = MD5.asHex(MD5.getHash(new java.io.File(uri)));
                        seq = (new XdmAtomicValue(hash)).getUnderlyingValue();
                    } catch(Exception e) {
                        System.err.println("ERR: ["+((StringValue) arguments[0].head()).getStringValue()+"]:"+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }
    
    // -----------------------------------------------------------------------
    // sx:fileSize
    // -----------------------------------------------------------------------

    public static final class FileSizeDefinition 
                        extends ExtensionFunctionDefinition {
        public StructuredQName getFunctionQName() {
            return new StructuredQName("sx", 
                                       "java:nl.mpi.tla.saxon", 
                                       "fileSize");
        }

        public int getMinimumNumberOfArguments() {
            return 1;
        }

        public int getMaximumNumberOfArguments() {
            return 1;
        }

        public SequenceType[] getArgumentTypes() {
            return new SequenceType[] { SequenceType.SINGLE_ANY_URI };
        }

        public SequenceType getResultType(SequenceType[] suppliedArgTypes) {
            return SequenceType.SINGLE_INTEGER;
        }
        
        public boolean dependsOnFocus() {
           return false;
        }

        public ExtensionFunctionCall makeCallExpression() {
            return new ExtensionFunctionCall() {
                @Override
                public Sequence call(XPathContext context, Sequence[] arguments) throws XPathException {
                    Sequence seq = null;
                    try {
                        URI uri = new URI(((StringValue) arguments[0].head()).getStringValue());
                        File file = new java.io.File(uri);
                        seq = (new XdmAtomicValue(file.length())).getUnderlyingValue();
                    } catch(Exception e) {
                        System.err.println("ERR: ["+((StringValue) arguments[0].head()).getStringValue()+"]:"+e.getMessage());
                        e.printStackTrace(System.err);
                    }
                    return seq;
                }
            };
        }
    }
}